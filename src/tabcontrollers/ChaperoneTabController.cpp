#include "ChaperoneTabController.h"
#include <QQuickWindow>
#include "../overlaycontroller.h"
#include <math.h>

// application namespace
namespace advsettings {


void ChaperoneTabController::initStage1() {
	m_height = getBoundsMaxY();

	auto settings = OverlayController::appSettings();
	settings->beginGroup("chaperoneSettings");
	m_enableChaperoneSwitchToBeginner = settings->value("chaperoneSwitchToBeginnerEnabled", false).toBool();
	m_chaperoneSwitchToBeginnerDistance = settings->value("chaperoneSwitchToBeginnerDistance", 0.5f).toFloat();
	m_enableChaperoneHapticFeedback = settings->value("chaperoneHapticFeedbackEnabled", false).toBool();
	m_chaperoneHapticFeedbackDistance = settings->value("chaperoneHapticFeedbackDistance", 0.5f).toFloat();
	m_enableChaperoneAlarmSound = settings->value("chaperoneAlarmSoundEnabled", false).toBool();
	m_chaperoneAlarmSoundLooping = settings->value("chaperoneAlarmSoundLooping", true).toBool();
	m_chaperoneAlarmSoundAdjustVolume = settings->value("chaperoneAlarmSoundAdjustVolume", false).toBool();
	m_chaperoneAlarmSoundDistance = settings->value("chaperoneAlarmSoundDistance", 0.5f).toFloat();
	m_enableChaperoneShowDashboard = settings->value("chaperoneShowDashboardEnabled", false).toBool();
	m_chaperoneShowDashboardDistance = settings->value("chaperoneShowDashboardDistance", 0.5f).toFloat();
	m_enableChaperoneVelocityModifier = settings->value("chaperoneVelocityModifierEnabled", false).toBool();
	m_chaperoneVelocityModifier = settings->value("chaperoneVelocityModifier", 0.3f).toFloat();
	m_chaperoneVelocityModifierCurrent = 1.0f;
	settings->endGroup();
	
	reloadChaperoneProfiles();
	eventLoopTick(nullptr, 0.0f, 0.0f, 0.0f);
}


void ChaperoneTabController::initStage2(OverlayController * parent, QQuickWindow * widget) {
	this->parent = parent;
	this->widget = widget;
}



ChaperoneTabController::~ChaperoneTabController() {
	m_chaperoneHapticFeedbackActive = false;
	if (m_chaperoneHapticFeedbackThread.joinable()) {
		m_chaperoneHapticFeedbackThread.join();
	}
}


void ChaperoneTabController::reloadChaperoneProfiles() {
	chaperoneProfiles.clear();
	auto settings = OverlayController::appSettings();
	settings->beginGroup("chaperoneSettings");
	auto profileCount = settings->beginReadArray("chaperoneProfiles");
	for (int i = 0; i < profileCount; i++) {
		settings->setArrayIndex(i);
		chaperoneProfiles.emplace_back();
		auto& entry = chaperoneProfiles[i];
		entry.profileName = settings->value("profileName").toString().toStdString();
		entry.includesChaperoneGeometry = settings->value("includesChaperoneGeometry", false).toBool();
		if (entry.includesChaperoneGeometry) {
			entry.chaperoneGeometryQuadCount = settings->value("chaperoneGeometryQuadCount", 0).toUInt();
			entry.chaperoneGeometryQuads.reset(new vr::HmdQuad_t[entry.chaperoneGeometryQuadCount]);
			unsigned qi = 0;
			for (auto q : settings->value("chaperoneGeometryQuads").toList()) {
				unsigned ci = 0;
				for (auto c : q.toList()) {
					unsigned cci = 0;
					for (auto cc : c.toList()) {
						entry.chaperoneGeometryQuads.get()[qi].vCorners[ci].v[cci] = cc.toFloat();
						cci++;
					}
					ci++;
				}
				qi++;
			}
			unsigned i1 = 0;
			for (auto l1 : settings->value("standingCenter").toList()) {
				unsigned i2 = 0;
				for (auto l2 : l1.toList()) {
					entry.standingCenter.m[i1][i2] = l2.toFloat();
					i2++;
				}
				i1++;
			}
			entry.playSpaceAreaX = settings->value("playSpaceAreaX", 0.0f).toFloat();
			entry.playSpaceAreaZ = settings->value("playSpaceAreaZ", 0.0f).toFloat();
		}
		entry.includesVisibility = settings->value("includesVisibility", false).toBool();
		if (entry.includesVisibility) {
			entry.visibility = settings->value("visibility", 0.6).toFloat();
		}
		entry.includesFadeDistance = settings->value("includesFadeDistance", false).toBool();
		if (entry.includesFadeDistance) {
			entry.fadeDistance = settings->value("fadeDistance", 0.7).toFloat();
		}
		entry.includesCenterMarker = settings->value("includesCenterMarker", false).toBool();
		if (entry.includesCenterMarker) {
			entry.centerMarker = settings->value("centerMarker", false).toBool();
		}
		entry.includesPlaySpaceMarker = settings->value("includesPlaySpaceMarker", false).toBool();
		if (entry.includesPlaySpaceMarker) {
			entry.playSpaceMarker = settings->value("playSpaceMarker", false).toBool();
		}
		entry.includesFloorBoundsMarker = settings->value("includesFloorBoundsMarker", false).toBool();
		if (entry.includesFloorBoundsMarker) {
			entry.floorBoundsMarker = settings->value("floorBoundsMarker", false).toBool();
		}
		entry.includesBoundsColor = settings->value("includesBoundsColor", false).toBool();
		if (entry.includesBoundsColor) {
			auto color = settings->value("boundsColor").toList();
			entry.boundsColor[0] = color[0].toInt();
			entry.boundsColor[1] = color[1].toInt();
			entry.boundsColor[2] = color[2].toInt();
		}
		entry.includesChaperoneStyle = settings->value("includesChaperoneStyle", false).toBool();
		if (entry.includesChaperoneStyle) {
			entry.chaperoneStyle = settings->value("chaperoneStyle", 0).toInt();
		}
		entry.includesForceBounds = settings->value("includesForceBounds", false).toBool();
		if (entry.includesForceBounds) {
			entry.forceBounds = settings->value("forceBounds", false).toBool();
		}
		entry.includesProximityWarningSettings = settings->value("includesProximityWarningSettings", false).toBool();
		if (entry.includesProximityWarningSettings){
			entry.enableChaperoneSwitchToBeginner = settings->value("chaperoneSwitchToBeginnerEnabled", false).toBool();
			entry.chaperoneSwitchToBeginnerDistance = settings->value("chaperoneSwitchToBeginnerDistance", 0.5f).toFloat();
			entry.enableChaperoneHapticFeedback = settings->value("chaperoneHapticFeedbackEnabled", false).toBool();
			entry.chaperoneHapticFeedbackDistance = settings->value("chaperoneHapticFeedbackDistance", 0.5f).toFloat();
			entry.enableChaperoneAlarmSound = settings->value("chaperoneAlarmSoundEnabled", false).toBool();
			entry.chaperoneAlarmSoundLooping = settings->value("chaperoneAlarmSoundLooping", true).toBool();
			entry.chaperoneAlarmSoundAdjustVolume = settings->value("chaperoneAlarmSoundAdjustVolume", false).toBool();
			entry.chaperoneAlarmSoundDistance = settings->value("chaperoneAlarmSoundDistance", 0.5f).toFloat();
			entry.enableChaperoneShowDashboard = settings->value("chaperoneShowDashboardEnabled", false).toBool();
			entry.chaperoneShowDashboardDistance = settings->value("chaperoneShowDashboardDistance", 0.5f).toFloat();
			entry.enableChaperoneVelocityModifier = settings->value("chaperoneVelocityModifierEnabled", false).toBool();
			entry.chaperoneVelocityModifier = settings->value("chaperoneVelocityModifier", 0.3f).toFloat();
		}
	}
	settings->endArray();
	settings->endGroup();
}

void ChaperoneTabController::saveChaperoneProfiles() {
	auto settings = OverlayController::appSettings();
	settings->beginGroup("chaperoneSettings");
	settings->beginWriteArray("chaperoneProfiles");
	unsigned i = 0;
	for (auto& p : chaperoneProfiles) {
		settings->setArrayIndex(i);
		settings->setValue("profileName", QString::fromStdString(p.profileName));
		settings->setValue("includesChaperoneGeometry", p.includesChaperoneGeometry);
		if (p.includesChaperoneGeometry) {
			settings->setValue("chaperoneGeometryQuadCount", p.chaperoneGeometryQuadCount);
			QVariantList quadList;
			for (unsigned qi = 0; qi < p.chaperoneGeometryQuadCount; qi++) {
				QVariantList cornerList;
				for (unsigned ci = 0; ci < 4; ci++) {
					QVariantList coordVector;
					for (unsigned cci = 0; cci < 3; cci++) {
						coordVector.push_back(p.chaperoneGeometryQuads.get()[qi].vCorners[ci].v[cci]);
					}
					cornerList.push_back(coordVector);
				}
				quadList.push_back(cornerList);
			}
			settings->setValue("chaperoneGeometryQuads", quadList);
			QVariantList l1;
			for (unsigned i1 = 0; i1 < 3; i1++) {
				QVariantList l2;
				for (unsigned i2 = 0; i2 < 4; i2++) {
					l2.push_back(p.standingCenter.m[i1][i2]);
				}
				l1.push_back(l2);
			}
			settings->setValue("standingCenter", l1);
			settings->setValue("playSpaceAreaX", p.playSpaceAreaX);
			settings->setValue("playSpaceAreaZ", p.playSpaceAreaZ);
		}
		settings->setValue("includesVisibility", p.includesVisibility);
		if (p.includesVisibility) {
			settings->setValue("visibility", p.visibility);
		}
		settings->setValue("includesFadeDistance", p.includesFadeDistance);
		if (p.includesFadeDistance) {
			settings->setValue("fadeDistance", p.fadeDistance);
		}
		settings->setValue("includesCenterMarker", p.includesCenterMarker);
		if (p.includesCenterMarker) {
			settings->setValue("centerMarker", p.centerMarker);
		}
		settings->setValue("includesPlaySpaceMarker", p.includesPlaySpaceMarker);
		if (p.includesPlaySpaceMarker) {
			settings->setValue("playSpaceMarker", p.playSpaceMarker);
		}
		settings->setValue("includesFloorBoundsMarker", p.includesFloorBoundsMarker);
		if (p.includesFloorBoundsMarker) {
			settings->setValue("floorBoundsMarker", p.floorBoundsMarker);
		}
		settings->setValue("includesBoundsColor", p.includesBoundsColor);
		if (p.includesBoundsColor) {
			QVariantList color;
			color.push_back(p.boundsColor[0]);
			color.push_back(p.boundsColor[1]);
			color.push_back(p.boundsColor[2]);
			settings->setValue("boundsColor", color);
		}
		settings->setValue("includesChaperoneStyle", p.includesChaperoneStyle);
		if (p.includesChaperoneStyle) {
			settings->setValue("chaperoneStyle", p.chaperoneStyle);
		}
		settings->setValue("includesForceBounds", p.includesForceBounds);
		if (p.includesForceBounds) {
			settings->setValue("forceBounds", p.forceBounds);
		}
		settings->setValue("includesProximityWarningSettings", p.includesProximityWarningSettings);
		if (p.includesProximityWarningSettings) {
			settings->setValue("chaperoneSwitchToBeginnerEnabled", p.enableChaperoneSwitchToBeginner);
			settings->setValue("chaperoneSwitchToBeginnerDistance", p.chaperoneSwitchToBeginnerDistance);
			settings->setValue("chaperoneHapticFeedbackEnabled", p.enableChaperoneHapticFeedback);
			settings->setValue("chaperoneHapticFeedbackDistance", p.chaperoneHapticFeedbackDistance);
			settings->setValue("chaperoneAlarmSoundEnabled", p.enableChaperoneAlarmSound);
			settings->setValue("chaperoneAlarmSoundLooping", p.chaperoneAlarmSoundLooping);
			settings->setValue("chaperoneAlarmSoundAdjustVolume", p.chaperoneAlarmSoundAdjustVolume);
			settings->setValue("chaperoneAlarmSoundDistance", p.chaperoneAlarmSoundDistance);
			settings->setValue("chaperoneShowDashboardEnabled", p.enableChaperoneShowDashboard);
			settings->setValue("chaperoneShowDashboardDistance", p.chaperoneShowDashboardDistance);
			settings->setValue("chaperoneVelocityModifierEnabled", p.enableChaperoneVelocityModifier);
			settings->setValue("chaperoneVelocityModifier", p.chaperoneVelocityModifier);
		}
		i++;
	}
	settings->endArray();
	settings->endGroup();
}


void ChaperoneTabController::handleChaperoneWarnings(float distance) {
	vr::VRControllerState_t hmdState;
	vr::VRSystem()->GetControllerState(vr::k_unTrackedDeviceIndex_Hmd, &hmdState, sizeof(vr::VRControllerState_t));

	// Switch to Beginner Mode
	if (m_enableChaperoneSwitchToBeginner) {
		float activationDistance = m_chaperoneSwitchToBeginnerDistance * m_chaperoneVelocityModifierCurrent;
		if (distance <= activationDistance && (hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor)) && !m_chaperoneSwitchToBeginnerActive) {
			vr::EVRSettingsError vrSettingsError;
			m_chaperoneSwitchToBeginnerLastStyle = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, &vrSettingsError);
			if (vrSettingsError != vr::VRSettingsError_None) {
				LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
			} else {
				vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, vr::COLLISION_BOUNDS_STYLE_BEGINNER, &vrSettingsError);
				if (vrSettingsError != vr::VRSettingsError_None) {
					LOG(WARNING) << "Could not set \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
				} else {
					vr::VRSettings()->Sync(true);
					m_chaperoneSwitchToBeginnerActive = true;
				}
			}
		} else if ((distance > activationDistance || !(hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor))) && m_chaperoneSwitchToBeginnerActive) {
			vr::EVRSettingsError vrSettingsError;
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, m_chaperoneSwitchToBeginnerLastStyle, &vrSettingsError);
			if (vrSettingsError != vr::VRSettingsError_None) {
				LOG(WARNING) << "Could not set \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
			} else {
				vr::VRSettings()->Sync(true);
				m_chaperoneSwitchToBeginnerActive = false;
			}
		}
	}

	// Haptic Feedback
	if (m_enableChaperoneHapticFeedback) {
		float activationDistance = m_chaperoneHapticFeedbackDistance * m_chaperoneVelocityModifierCurrent;
		if (distance <= activationDistance && (hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor))) {
			if (!m_chaperoneHapticFeedbackActive) {
				if (m_chaperoneHapticFeedbackThread.joinable()) {
					m_chaperoneHapticFeedbackActive = false;
					m_chaperoneHapticFeedbackThread.join();
				}
				m_chaperoneHapticFeedbackActive = true;
				m_chaperoneHapticFeedbackThread = std::thread([](ChaperoneTabController* _this) {
					auto leftIndex = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
					auto rightIndex = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
					while (_this->m_chaperoneHapticFeedbackActive) {
						if (leftIndex != vr::k_unTrackedDeviceIndexInvalid) {
							vr::VRSystem()->TriggerHapticPulse(leftIndex, 0, 2000);
						}
						if (rightIndex != vr::k_unTrackedDeviceIndexInvalid) {
							vr::VRSystem()->TriggerHapticPulse(rightIndex, 0, 2000);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(5));
					}
				}, this);
			}
		} else if ((distance > activationDistance || !(hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor))) && m_chaperoneHapticFeedbackActive) {
			m_chaperoneHapticFeedbackActive = false;
		}
	}

	// Alarm Sound
	if (m_enableChaperoneAlarmSound) {
		float activationDistance = m_chaperoneAlarmSoundDistance * m_chaperoneVelocityModifierCurrent;
		if (distance <= activationDistance && (hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor))) {
			if (!m_chaperoneAlarmSoundActive) {
				parent->playAlarm01Sound(m_chaperoneAlarmSoundLooping);
				m_chaperoneAlarmSoundActive = true;
			}
			if (m_chaperoneAlarmSoundLooping && m_chaperoneAlarmSoundAdjustVolume) {
				float vol = 1.1f - distance / activationDistance;
				if (vol > 1.0f) {
					vol = 1.0f;
				}
				parent->setAlarm01SoundVolume(vol);
			} else {
				parent->setAlarm01SoundVolume(1.0f);
			}
		} else if ((distance > activationDistance || !(hmdState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ProximitySensor))) && m_chaperoneAlarmSoundActive) {
			parent->cancelAlarm01Sound();
			m_chaperoneAlarmSoundActive = false;
		}
	}

	// Show Dashboard
	if (m_enableChaperoneShowDashboard) {
		float activationDistance = m_chaperoneShowDashboardDistance * m_chaperoneVelocityModifierCurrent;
		if (distance <= activationDistance && !m_chaperoneShowDashboardActive) {
			if (!vr::VROverlay()->IsDashboardVisible()) {
				vr::VROverlay()->ShowDashboard(OverlayController::applicationKey);
			}
			m_chaperoneShowDashboardActive = true;
		} else if (distance > activationDistance && m_chaperoneShowDashboardActive) {
			m_chaperoneShowDashboardActive = false;
		}
	}
}


void ChaperoneTabController::eventLoopTick(vr::TrackedDevicePose_t* devicePoses, float leftSpeed, float rightSpeed, float hmdSpeed) {
	m_chaperoneVelocityModifierCurrent = 1.0f;
	if (m_enableChaperoneVelocityModifier) {
		float mod = m_chaperoneVelocityModifier * std::max({ leftSpeed, rightSpeed, hmdSpeed });
		if (mod > 0.02) {
			m_chaperoneVelocityModifierCurrent += mod;
		}
	}
	//LOG(INFO) << "m_chaperoneVelocityModifierCurrent: " << m_chaperoneVelocityModifierCurrent;
	float newFadeDistance = m_fadeDistance * m_chaperoneVelocityModifierCurrent;
	if (m_fadeDistanceModified != newFadeDistance) {
		m_fadeDistanceModified = newFadeDistance;
		vr::VRSettings()->SetFloat(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_FadeDistance_Float, m_fadeDistanceModified);
		vr::VRSettings()->Sync();
	}

	if (devicePoses) {
		std::lock_guard<std::recursive_mutex> lock(parent->chaperoneUtils().mutex());
		auto minDistance = NAN;
		auto& poseHmd = devicePoses[vr::k_unTrackedDeviceIndex_Hmd];
		if (poseHmd.bPoseIsValid && poseHmd.bDeviceIsConnected && poseHmd.eTrackingResult == vr::TrackingResult_Running_OK) {
			auto distanceHmd = parent->chaperoneUtils().getDistanceToChaperone({
				poseHmd.mDeviceToAbsoluteTracking.m[0][3],
				poseHmd.mDeviceToAbsoluteTracking.m[1][3],
				poseHmd.mDeviceToAbsoluteTracking.m[2][3]
			});
			if (!isnan(distanceHmd)) {
				minDistance = distanceHmd;
			}
		}
		auto leftIndex = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
		if (leftIndex != vr::k_unTrackedDeviceIndexInvalid) {
			auto& poseLeft = devicePoses[leftIndex];
			if (poseLeft.bPoseIsValid && poseLeft.bDeviceIsConnected && poseLeft.eTrackingResult == vr::TrackingResult_Running_OK) {
				auto distanceLeft = parent->chaperoneUtils().getDistanceToChaperone({
					poseLeft.mDeviceToAbsoluteTracking.m[0][3],
					poseLeft.mDeviceToAbsoluteTracking.m[1][3],
					poseLeft.mDeviceToAbsoluteTracking.m[2][3]
				});
				if (!isnan(distanceLeft) && (isnan(minDistance) || distanceLeft < minDistance)) {
					minDistance = distanceLeft;
				}
			}
		}
		auto rightIndex = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
		if (rightIndex != vr::k_unTrackedDeviceIndexInvalid) {
			auto& poseRight = devicePoses[rightIndex];
			if (poseRight.bPoseIsValid && poseRight.bDeviceIsConnected && poseRight.eTrackingResult == vr::TrackingResult_Running_OK) {
				auto distanceRight = parent->chaperoneUtils().getDistanceToChaperone({
					poseRight.mDeviceToAbsoluteTracking.m[0][3],
					poseRight.mDeviceToAbsoluteTracking.m[1][3],
					poseRight.mDeviceToAbsoluteTracking.m[2][3]
				});
				if (!isnan(distanceRight) && (isnan(minDistance) || distanceRight < minDistance)) {
					minDistance = distanceRight;
				}
			}
		}
		if (!isnan(minDistance)) {
			handleChaperoneWarnings(minDistance);
		}
	}

	if (settingsUpdateCounter >= 50) {
		vr::EVRSettingsError vrSettingsError;
		float vis = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaA_Int32, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_ColorGammaA_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
		setBoundsVisibility(vis/255.0f);
		if (m_chaperoneVelocityModifierCurrent == 1.0f) {
			auto fd = vr::VRSettings()->GetFloat(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_FadeDistance_Float, &vrSettingsError);
			if (vrSettingsError != vr::VRSettingsError_None) {
				LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_FadeDistance_Float << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
			}
			setFadeDistance(fd);
		}
		auto cm = vr::VRSettings()->GetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_CenterMarkerOn_Bool, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_CenterMarkerOn_Bool << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
		setCenterMarker(cm);
		auto ps = vr::VRSettings()->GetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_PlaySpaceOn_Bool, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_PlaySpaceOn_Bool << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
		setPlaySpaceMarker(ps);
		updateHeight(getBoundsMaxY());
		settingsUpdateCounter = 0;
	} else {
		settingsUpdateCounter++;
	}
}


float ChaperoneTabController::boundsVisibility() const {
	return m_visibility;
}


void ChaperoneTabController::setBoundsVisibility(float value, bool notify) {
	if (m_visibility != value) {
		m_visibility = value;
		vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaA_Int32, 255 * m_visibility);
		vr::VRSettings()->Sync();
		if (notify) {
			emit boundsVisibilityChanged(m_visibility);
		}
	}
}


float ChaperoneTabController::fadeDistance() const {
	return m_fadeDistance;
}


void ChaperoneTabController::setFadeDistance(float value, bool notify) {
	if (m_fadeDistance != value) {
		m_fadeDistance = value;
		vr::VRSettings()->SetFloat(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_FadeDistance_Float, m_fadeDistance);
		vr::VRSettings()->Sync();
		if (notify) {
			emit fadeDistanceChanged(m_fadeDistance);
		}
	}
}


float ChaperoneTabController::height() const {
	return m_height;
}


void ChaperoneTabController::setHeight(float value, bool notify) {
	if (m_height != value) {
		m_height = value;
		vr::VRChaperoneSetup()->RevertWorkingCopy();
		unsigned collisionBoundsCount = 0;
		vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(nullptr, &collisionBoundsCount);
		if (collisionBoundsCount > 0) {
			vr::HmdQuad_t* collisionBounds = new vr::HmdQuad_t[collisionBoundsCount];
			vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(collisionBounds, &collisionBoundsCount);
			for (unsigned b = 0; b < collisionBoundsCount; b++) {
				collisionBounds[b].vCorners[0].v[1] = 0.0;
				collisionBounds[b].vCorners[1].v[1] = value;
				collisionBounds[b].vCorners[2].v[1] = value;
				collisionBounds[b].vCorners[3].v[1] = 0.0;
			}
			vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(collisionBounds, collisionBoundsCount);
			vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
			delete collisionBounds;
		}
		if (notify) {
			emit heightChanged(m_fadeDistance);
		}
	}
}


void ChaperoneTabController::updateHeight(float value, bool notify) {
	if (m_height != value) {
		m_height = value;
		if (notify) {
			emit heightChanged(m_fadeDistance);
		}
	}
}


bool ChaperoneTabController::centerMarker() const {
	return m_centerMarker;
}

void ChaperoneTabController::setCenterMarker(bool value, bool notify) {
	if (m_centerMarker != value) {
		m_centerMarker = value;
		vr::VRSettings()->SetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_CenterMarkerOn_Bool, m_centerMarker);
		vr::VRSettings()->Sync();
		if (notify) {
			emit centerMarkerChanged(m_centerMarker);
		}
	}
}

bool ChaperoneTabController::playSpaceMarker() const {
	return m_playSpaceMarker;
}

void ChaperoneTabController::setPlaySpaceMarker(bool value, bool notify) {
	if (m_playSpaceMarker != value) {
		m_playSpaceMarker = value;
		vr::VRSettings()->SetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_PlaySpaceOn_Bool, m_playSpaceMarker);
		vr::VRSettings()->Sync();
		if (notify) {
			emit playSpaceMarkerChanged(m_playSpaceMarker);
		}
	}
}

bool ChaperoneTabController::forceBounds() const {
	return m_forceBounds;
}


bool ChaperoneTabController::isChaperoneSwitchToBeginnerEnabled() const {
	return m_enableChaperoneSwitchToBeginner;
}

float ChaperoneTabController::chaperoneSwitchToBeginnerDistance() const {
	return m_chaperoneSwitchToBeginnerDistance;
}


bool ChaperoneTabController::isChaperoneHapticFeedbackEnabled() const {
	return m_enableChaperoneHapticFeedback;
}

float ChaperoneTabController::chaperoneHapticFeedbackDistance() const {
	return m_chaperoneHapticFeedbackDistance;
}


bool ChaperoneTabController::isChaperoneAlarmSoundEnabled() const {
	return m_enableChaperoneAlarmSound;
}

bool ChaperoneTabController::isChaperoneAlarmSoundLooping() const {
	return m_chaperoneAlarmSoundLooping;
}

bool ChaperoneTabController::isChaperoneAlarmSoundAdjustVolume() const {
	return m_chaperoneAlarmSoundAdjustVolume;
}

float ChaperoneTabController::chaperoneAlarmSoundDistance() const {
	return m_chaperoneAlarmSoundDistance;
}


bool ChaperoneTabController::isChaperoneShowDashboardEnabled() const {
	return m_enableChaperoneShowDashboard;
}

float ChaperoneTabController::chaperoneShowDashboardDistance() const {
	return m_chaperoneShowDashboardDistance;
}


bool ChaperoneTabController::isChaperoneVelocityModifierEnabled() const {
	return m_enableChaperoneVelocityModifier;
}

float ChaperoneTabController::chaperoneVelocityModifier() const {
	return m_chaperoneVelocityModifier;
}




Q_INVOKABLE unsigned ChaperoneTabController::getChaperoneProfileCount() {
	return (unsigned)chaperoneProfiles.size();
}

Q_INVOKABLE QString ChaperoneTabController::getChaperoneProfileName(unsigned index) {
	if (index >= chaperoneProfiles.size()) {
		return QString();
	} else {
		return QString::fromStdString(chaperoneProfiles[index].profileName);
	}
}

void ChaperoneTabController::setForceBounds(bool value, bool notify) {
	if (m_forceBounds != value) {
		m_forceBounds = value;
		vr::VRChaperone()->ForceBoundsVisible(m_forceBounds);
		if (notify) {
			emit forceBoundsChanged(m_forceBounds);
		}
	}
}


void ChaperoneTabController::setChaperoneSwitchToBeginnerEnabled(bool value, bool notify) {
	if (m_enableChaperoneSwitchToBeginner != value) {
		if (!value && m_chaperoneSwitchToBeginnerActive) {
			vr::EVRSettingsError vrSettingsError;
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, m_chaperoneSwitchToBeginnerLastStyle, &vrSettingsError);
			if (vrSettingsError != vr::VRSettingsError_None) {
				LOG(WARNING) << "Could not set \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
			} else {
				vr::VRSettings()->Sync(true);
			}
		}
		m_enableChaperoneSwitchToBeginner = value;
		m_chaperoneSwitchToBeginnerActive = false;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneSwitchToBeginnerEnabled", m_enableChaperoneSwitchToBeginner);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneSwitchToBeginnerEnabledChanged(m_enableChaperoneSwitchToBeginner);
		}
	}
}

void ChaperoneTabController::setChaperoneSwitchToBeginnerDistance(float value, bool notify) {
	if (m_chaperoneSwitchToBeginnerDistance != value) {
		m_chaperoneSwitchToBeginnerDistance = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneSwitchToBeginnerDistance", m_chaperoneSwitchToBeginnerDistance);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneSwitchToBeginnerDistanceChanged(m_chaperoneSwitchToBeginnerDistance);
		}
	}
}


void ChaperoneTabController::setChaperoneHapticFeedbackEnabled(bool value, bool notify) {
	if (m_enableChaperoneHapticFeedback != value) {
		m_chaperoneHapticFeedbackActive = false;
		if (m_chaperoneHapticFeedbackThread.joinable()) {
			m_chaperoneHapticFeedbackThread.join();
		}
		m_enableChaperoneHapticFeedback = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneHapticFeedbackEnabled", m_enableChaperoneHapticFeedback);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneHapticFeedbackEnabledChanged(m_enableChaperoneHapticFeedback);
		}
	}
}

void ChaperoneTabController::setChaperoneHapticFeedbackDistance(float value, bool notify) {
	if (m_chaperoneHapticFeedbackDistance != value) {
		m_chaperoneHapticFeedbackDistance = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneHapticFeedbackDistance", m_chaperoneHapticFeedbackDistance);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneHapticFeedbackDistanceChanged(m_chaperoneHapticFeedbackDistance);
		}
	}
}


void ChaperoneTabController::setChaperoneAlarmSoundEnabled(bool value, bool notify) {
	if (m_enableChaperoneAlarmSound != value) {
		if (!value && m_chaperoneAlarmSoundActive) {
			parent->cancelAlarm01Sound();
		}
		m_enableChaperoneAlarmSound = value;
		m_chaperoneAlarmSoundActive = false;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneAlarmSoundEnabled", m_enableChaperoneAlarmSound);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneAlarmSoundEnabledChanged(m_enableChaperoneAlarmSound);
		}
	}
}

void ChaperoneTabController::setChaperoneAlarmSoundLooping(bool value, bool notify) {
	if (m_chaperoneAlarmSoundLooping != value) {
		m_chaperoneAlarmSoundLooping = value;
		if (m_enableChaperoneAlarmSound && m_chaperoneAlarmSoundActive) {
			if (m_chaperoneAlarmSoundLooping) {
				parent->playAlarm01Sound(m_chaperoneAlarmSoundLooping);
			} else {
				parent->cancelAlarm01Sound();
			}
		}
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneAlarmSoundLooping", m_chaperoneAlarmSoundLooping);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneAlarmSoundLoopingChanged(m_chaperoneAlarmSoundLooping);
		}
	}
}

void ChaperoneTabController::setChaperoneAlarmSoundAdjustVolume(bool value, bool notify) {
	if (m_chaperoneAlarmSoundAdjustVolume != value) {
		m_chaperoneAlarmSoundAdjustVolume = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneAlarmSoundAdjustVolume", m_chaperoneAlarmSoundAdjustVolume);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneAlarmSoundAdjustVolumeChanged(m_chaperoneAlarmSoundAdjustVolume);
		}
	}
}

void ChaperoneTabController::setChaperoneAlarmSoundDistance(float value, bool notify) {
	if (m_chaperoneAlarmSoundDistance != value) {
		m_chaperoneAlarmSoundDistance = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneAlarmSoundDistance", m_chaperoneAlarmSoundDistance);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneAlarmSoundDistanceChanged(m_chaperoneAlarmSoundDistance);
		}
	}
}


void ChaperoneTabController::setChaperoneShowDashboardEnabled(bool value, bool notify) {
	if (m_enableChaperoneShowDashboard != value) {
		m_enableChaperoneShowDashboard = value;
		m_chaperoneShowDashboardActive = false;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneShowDashboardEnabled", m_enableChaperoneShowDashboard);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneShowDashboardEnabledChanged(m_enableChaperoneShowDashboard);
		}
	}
}

void ChaperoneTabController::setChaperoneShowDashboardDistance(float value, bool notify) {
	if (m_chaperoneShowDashboardDistance != value) {
		m_chaperoneShowDashboardDistance = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneShowDashboardDistance", m_chaperoneShowDashboardDistance);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneShowDashboardDistanceChanged(m_chaperoneShowDashboardDistance);
		}
	}
}


void ChaperoneTabController::setChaperoneVelocityModifierEnabled(bool value, bool notify) {
	if (m_enableChaperoneVelocityModifier != value) {
		m_enableChaperoneVelocityModifier = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneVelocityModifierEnabled", m_enableChaperoneVelocityModifier);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneVelocityModifierEnabledChanged(m_enableChaperoneVelocityModifier);
		}
	}
}

void ChaperoneTabController::setChaperoneVelocityModifier(float value, bool notify) {
	if (m_chaperoneVelocityModifier != value) {
		m_chaperoneVelocityModifier = value;
		auto settings = OverlayController::appSettings();
		settings->beginGroup("chaperoneSettings");
		settings->setValue("chaperoneVelocityModifier", m_chaperoneVelocityModifier);
		settings->endGroup();
		settings->sync();
		if (notify) {
			emit chaperoneVelocityModifierChanged(m_chaperoneVelocityModifier);
		}
	}
}



void ChaperoneTabController::flipOrientation() {
	parent->RotateUniverseCenter(vr::TrackingUniverseStanding, (float)M_PI);
}

void ChaperoneTabController::reloadFromDisk() {
	vr::VRChaperoneSetup()->ReloadFromDisk(vr::EChaperoneConfigFile_Live);
	vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
}

void ChaperoneTabController::addChaperoneProfile(QString name, bool includeGeometry, bool includeVisbility, bool includeFadeDistance, bool includeCenterMarker,
		bool includePlaySpaceMarker, bool includeFloorBounds, bool includeBoundsColor, bool includeChaperoneStyle, bool includeForceBounds, bool includesProximityWarningSettings) {
	vr::EVRSettingsError vrSettingsError;
	ChaperoneProfile* profile = nullptr;
	for (auto& p : chaperoneProfiles) {
		if (p.profileName.compare(name.toStdString()) == 0) {
			profile = &p;
			break;
		}
	}
	if (!profile) {
		auto i = chaperoneProfiles.size();
		chaperoneProfiles.emplace_back();
		profile = &chaperoneProfiles[i];
	}
	profile->profileName = name.toStdString();
	profile->includesChaperoneGeometry = includeGeometry;
	if (includeGeometry) {
		vr::VRChaperoneSetup()->RevertWorkingCopy();
		uint32_t quadCount = 0;
		vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &quadCount);
		profile->chaperoneGeometryQuadCount = quadCount;
		profile->chaperoneGeometryQuads.reset(new vr::HmdQuad_t[quadCount]);
		vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(profile->chaperoneGeometryQuads.get(), &quadCount);
		vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&profile->standingCenter);
		vr::VRChaperoneSetup()->GetWorkingPlayAreaSize(&profile->playSpaceAreaX, &profile->playSpaceAreaZ);
	}
	profile->includesVisibility = includeVisbility;
	if (includeVisbility) {
		profile->visibility = m_visibility;
	}
	profile->includesFadeDistance = includeFadeDistance;
	if (includeFadeDistance) {
		profile->fadeDistance = m_fadeDistance;
	}
	profile->includesCenterMarker = includeCenterMarker;
	if (includeCenterMarker) {
		profile->centerMarker = m_centerMarker;
	}
	profile->includesPlaySpaceMarker = includePlaySpaceMarker;
	if (includePlaySpaceMarker) {
		profile->playSpaceMarker = m_playSpaceMarker;
	}
	profile->includesFloorBoundsMarker = includeFloorBounds;
	if (includeFloorBounds) {
		profile->floorBoundsMarker = vr::VRSettings()->GetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_GroundPerimeterOn_Bool, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_GroundPerimeterOn_Bool << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
	}
	profile->includesBoundsColor = includeBoundsColor;
	if (includeBoundsColor) {
		profile->boundsColor[0] = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaR_Int32, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_ColorGammaR_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
		profile->boundsColor[1] = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaG_Int32, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_ColorGammaG_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
		profile->boundsColor[2] = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaB_Int32, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_ColorGammaB_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
	}
	profile->includesChaperoneStyle = includeChaperoneStyle;
	if (includeChaperoneStyle) {
		profile->chaperoneStyle = vr::VRSettings()->GetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not read \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		}
	}
	profile->includesForceBounds = includeForceBounds;
	if (includeForceBounds) {
		profile->forceBounds = m_forceBounds;
	}
	profile->includesProximityWarningSettings = includesProximityWarningSettings;
	if (includesProximityWarningSettings) {
		profile->enableChaperoneSwitchToBeginner = m_enableChaperoneSwitchToBeginner;
		profile->chaperoneSwitchToBeginnerDistance = m_chaperoneSwitchToBeginnerDistance;
		profile->enableChaperoneHapticFeedback = m_enableChaperoneHapticFeedback;
		profile->chaperoneHapticFeedbackDistance = m_chaperoneHapticFeedbackDistance;
		profile->enableChaperoneAlarmSound = m_enableChaperoneAlarmSound;
		profile->chaperoneAlarmSoundLooping = m_chaperoneAlarmSoundLooping;
		profile->chaperoneAlarmSoundAdjustVolume = m_chaperoneAlarmSoundAdjustVolume;
		profile->chaperoneAlarmSoundDistance = m_chaperoneAlarmSoundDistance;
		profile->enableChaperoneShowDashboard = m_enableChaperoneShowDashboard;
		profile->chaperoneShowDashboardDistance = m_chaperoneShowDashboardDistance;
		profile->enableChaperoneVelocityModifier = m_enableChaperoneVelocityModifier;
		profile->chaperoneVelocityModifier = m_chaperoneVelocityModifier;
	}
	saveChaperoneProfiles();
	OverlayController::appSettings()->sync();
	emit chaperoneProfilesUpdated();
}

void ChaperoneTabController::applyChaperoneProfile(unsigned index) {
	if (index < chaperoneProfiles.size()) {
		auto& profile = chaperoneProfiles[index];
		if (profile.includesChaperoneGeometry) {
			vr::VRChaperoneSetup()->RevertWorkingCopy();
			vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(profile.chaperoneGeometryQuads.get(), profile.chaperoneGeometryQuadCount);
			vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&profile.standingCenter);
			vr::VRChaperoneSetup()->SetWorkingPlayAreaSize(profile.playSpaceAreaX, profile.playSpaceAreaZ);
			vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
		}
		if (profile.includesVisibility) {
			setBoundsVisibility(profile.visibility);
		}
		if (profile.includesFadeDistance) {
			setFadeDistance(profile.fadeDistance);
		}
		if (profile.includesCenterMarker) {
			setCenterMarker(profile.centerMarker);
		}
		if (profile.includesPlaySpaceMarker) {
			setPlaySpaceMarker(profile.playSpaceMarker);
		}
		if (profile.includesFloorBoundsMarker) {
			vr::VRSettings()->SetBool(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_GroundPerimeterOn_Bool, profile.floorBoundsMarker);
		}
		if (profile.includesBoundsColor) {
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaR_Int32, profile.boundsColor[0]);
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaG_Int32, profile.boundsColor[1]);
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaB_Int32, profile.boundsColor[2]);
		}
		if (profile.includesChaperoneStyle) {
			vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, profile.chaperoneStyle);
		}
		if (profile.includesForceBounds) {
			setForceBounds(profile.forceBounds);
		}
		if (profile.includesProximityWarningSettings) {
			setChaperoneSwitchToBeginnerDistance(profile.chaperoneSwitchToBeginnerDistance);
			setChaperoneSwitchToBeginnerEnabled(profile.enableChaperoneSwitchToBeginner);
			setChaperoneHapticFeedbackDistance(profile.chaperoneHapticFeedbackDistance);
			setChaperoneHapticFeedbackEnabled(profile.enableChaperoneHapticFeedback);
			setChaperoneAlarmSoundLooping(profile.chaperoneAlarmSoundLooping);
			setChaperoneAlarmSoundAdjustVolume(profile.chaperoneAlarmSoundAdjustVolume);
			setChaperoneAlarmSoundDistance(profile.chaperoneAlarmSoundDistance);
			setChaperoneAlarmSoundEnabled(profile.enableChaperoneAlarmSound);
			setChaperoneShowDashboardDistance(profile.chaperoneShowDashboardDistance);
			setChaperoneShowDashboardEnabled(profile.enableChaperoneShowDashboard);
			setChaperoneVelocityModifier(profile.chaperoneVelocityModifier);
			setChaperoneVelocityModifierEnabled(profile.enableChaperoneVelocityModifier);
		}
		vr::VRSettings()->Sync(true);
		updateHeight(getBoundsMaxY());
	}
}

void ChaperoneTabController::deleteChaperoneProfile(unsigned index) {
	if (index < chaperoneProfiles.size()) {
		auto pos = chaperoneProfiles.begin() + index;
		chaperoneProfiles.erase(pos);
		saveChaperoneProfiles();
		OverlayController::appSettings()->sync();
		emit chaperoneProfilesUpdated();
	}
}

float ChaperoneTabController::getBoundsMaxY() {
	unsigned collisionBoundsCount = 0;
	float result = FP_NAN;
	vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(nullptr, &collisionBoundsCount);
	if (collisionBoundsCount > 0) {
		vr::HmdQuad_t* collisionBounds = new vr::HmdQuad_t[collisionBoundsCount];
		vr::VRChaperoneSetup()->GetLiveCollisionBoundsInfo(collisionBounds, &collisionBoundsCount);
		for (unsigned b = 0; b < collisionBoundsCount; b++) {
			int ci;
			if (collisionBounds[b].vCorners[1].v[1] >= collisionBounds[b].vCorners[2].v[1]) {
				ci = 1;
			} else {
				ci = 2;
			}
			if (isnan(result) || result < collisionBounds[b].vCorners[ci].v[1]) {
				result = collisionBounds[b].vCorners[ci].v[1];
			}
		}
		delete collisionBounds;
	}
	return result;
}

void ChaperoneTabController::reset() {
	vr::EVRSettingsError vrSettingsError;

	vr::VRSettings()->RemoveKeyInSection(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaA_Int32, &vrSettingsError);
	if (vrSettingsError != vr::VRSettingsError_None) {
		LOG(WARNING) << "Could not remove \"" << vr::k_pch_CollisionBounds_ColorGammaA_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
	}

	vr::VRSettings()->RemoveKeyInSection(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_FadeDistance_Float, &vrSettingsError);
	if (vrSettingsError != vr::VRSettingsError_None) {
		LOG(WARNING) << "Could not remove \"" << vr::k_pch_CollisionBounds_FadeDistance_Float << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
	}

	vr::VRSettings()->RemoveKeyInSection(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_CenterMarkerOn_Bool, &vrSettingsError);
	if (vrSettingsError != vr::VRSettingsError_None) {
		LOG(WARNING) << "Could not remove \"" << vr::k_pch_CollisionBounds_CenterMarkerOn_Bool << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
	}

	vr::VRSettings()->RemoveKeyInSection(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_PlaySpaceOn_Bool, &vrSettingsError);
	if (vrSettingsError != vr::VRSettingsError_None) {
		LOG(WARNING) << "Could not remove \"" << vr::k_pch_CollisionBounds_PlaySpaceOn_Bool << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
	}

	setForceBounds(false);

	vr::VRSettings()->Sync();
	settingsUpdateCounter = 999; // Easiest way to get default values
}


void ChaperoneTabController::shutdown() {
	if (m_enableChaperoneSwitchToBeginner && m_chaperoneSwitchToBeginnerActive) {
		vr::EVRSettingsError vrSettingsError;
		vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_Style_Int32, m_chaperoneSwitchToBeginnerLastStyle, &vrSettingsError);
		if (vrSettingsError != vr::VRSettingsError_None) {
			LOG(WARNING) << "Could not set \"" << vr::k_pch_CollisionBounds_Style_Int32 << "\" setting: " << vr::VRSettings()->GetSettingsErrorNameFromEnum(vrSettingsError);
		} else {
			vr::VRSettings()->Sync(true);
		}
	}
}


} // namespace advconfig
