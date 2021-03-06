/*
 * Copyright Regents of the University of Minnesota, 2015.  This software is released under the following license: http://opensource.org/licenses/GPL-2.0
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Ben Knorlein
 */

#include <ctime>
#include <cctype>

#include "VROpenVRRenderModelHandler.h"
#include "VROpenVRInputDevice.h"
#include "VROpenVRNode.h"

namespace MinVR {
                       
	VROpenVRInputDevice::VROpenVRInputDevice(vr::IVRSystem *pHMD, string name, VROpenVRNode * node, unsigned char openvr_plugin_flags) :m_pHMD(pHMD), m_name(name), m_node(node),
		m_report_state(openvr_plugin_flags), m_report_state_touched(openvr_plugin_flags & Touched), m_report_state_pressed(openvr_plugin_flags & Pressed), m_report_state_axis(openvr_plugin_flags & Axis), m_report_state_pose(openvr_plugin_flags & Pose)
	{
		updateDeviceNames();

		for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		{
			setTipOffset(unDevice);
		}
	}

	VROpenVRInputDevice::~VROpenVRInputDevice() {
	}

void VROpenVRInputDevice::appendNewInputEventsSinceLastCall(VRDataQueue* queue) {
	//process VR Events
	vr::VREvent_t event;
	vr::TrackedDevicePose_t pose;
	while( m_pHMD->PollNextEventWithPose(vr::VRCompositor()->GetTrackingSpace(),  &event, sizeof(event), &pose ) )
	{
		processVREvent( event, &pose );
	}

	//Report current States
	reportStates();

    for (int f = 0; f < _events.size(); f++)
	{
    	queue->push(_events[f]);
    }
    _events.clear();
}

void VROpenVRInputDevice::updatePoses(){
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (m_pHMD->IsTrackedDeviceConnected(unDevice)){
			std::string event_name = getDeviceName(unDevice);
			_dataIndex.addData(event_name + "/Pose", poseToMatrix4(&m_rTrackedDevicePose[unDevice]) * m_tip_offset[unDevice]);
			_events.push_back(_dataIndex.serialize(event_name));
		}
	}
}

VRMatrix4 VROpenVRInputDevice::getPose(int device_idx){
	return poseToMatrix4( &m_rTrackedDevicePose[device_idx]);
}

bool getButtonState(vr::EVRButtonId id, uint64_t state){
	return vr::ButtonMaskFromId(id) & state;
}

std::string VROpenVRInputDevice::getAxisType(int device, int axis){
	int32_t type = m_pHMD->GetInt32TrackedDeviceProperty( device, ((vr::ETrackedDeviceProperty) (vr::Prop_Axis0Type_Int32 +  axis)));
	switch(type){
		case vr::k_eControllerAxis_None:
			return "None";
		case vr::k_eControllerAxis_TrackPad:
			return "TrackPad";
		case vr::k_eControllerAxis_Joystick:
			return "Joystick";
		case vr::k_eControllerAxis_Trigger:
			return "Trigger";
	}
	return "Invalid";
}

void VROpenVRInputDevice::reportStates(){
	if (m_report_state){
		for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		{
			if (m_pHMD->IsTrackedDeviceConnected(unDevice)){
				vr::VRControllerState_t state;
				vr::TrackedDevicePose_t pose;
				std::string event_name = getDeviceName(unDevice);
				m_pHMD->GetControllerStateWithPose(vr::VRCompositor()->GetTrackingSpace(), unDevice, &state, sizeof(state), &pose);//){

				if (m_report_state_pose){
					_dataIndex.addData(event_name + "/State/Pose", poseToMatrix4(&pose) * m_tip_offset[unDevice]);
				}
				if (m_pHMD->GetTrackedDeviceClass(unDevice) == vr::TrackedDeviceClass_Controller){
					
					if (m_report_state_pressed){
						_dataIndex.addData(event_name + "/State/" + "SystemButton" + "_Pressed", getButtonState(vr::k_EButton_System, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "ApplicationMenuButton" + "_Pressed", getButtonState(vr::k_EButton_ApplicationMenu, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "GripButton" + "_Pressed", getButtonState(vr::k_EButton_Grip, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_LeftButton" + "_Pressed", getButtonState(vr::k_EButton_DPad_Left, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_UpButton" + "_Pressed", getButtonState(vr::k_EButton_DPad_Up, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_RightButton" + "_Pressed", getButtonState(vr::k_EButton_DPad_Right, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_DownButton" + "_Pressed", getButtonState(vr::k_EButton_DPad_Down, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "AButton" + "_Pressed", getButtonState(vr::k_EButton_A, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis0Button" + "_Pressed", getButtonState(vr::k_EButton_Axis0, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis1Button" + "_Pressed", getButtonState(vr::k_EButton_Axis1, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis2Button" + "_Pressed", getButtonState(vr::k_EButton_Axis2, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis3Button" + "_Pressed", getButtonState(vr::k_EButton_Axis3, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis4Button" + "_Pressed", getButtonState(vr::k_EButton_Axis4, state.ulButtonPressed));
					}

					if (m_report_state_touched){
						_dataIndex.addData(event_name + "/State/" + "SystemButton" + "_Touched", getButtonState(vr::k_EButton_System, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "ApplicationMenuButton" + "_Touched", getButtonState(vr::k_EButton_ApplicationMenu, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "GripButton" + "_Touched", getButtonState(vr::k_EButton_Grip, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_LeftButton" + "_Touched", getButtonState(vr::k_EButton_DPad_Left, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_UpButton" + "_Touched", getButtonState(vr::k_EButton_DPad_Up, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_RightButton" + "_Touched", getButtonState(vr::k_EButton_DPad_Right, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "DPad_DownButton" + "_Touched", getButtonState(vr::k_EButton_DPad_Down, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "AButton" + "_Touched", getButtonState(vr::k_EButton_A, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis0Button" + "_Touched", getButtonState(vr::k_EButton_Axis0, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis1Button" + "_Touched", getButtonState(vr::k_EButton_Axis1, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis2Button" + "_Touched", getButtonState(vr::k_EButton_Axis2, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis3Button" + "_Touched", getButtonState(vr::k_EButton_Axis3, state.ulButtonPressed));
						_dataIndex.addData(event_name + "/State/" + "Axis4Button" + "_Touched", getButtonState(vr::k_EButton_Axis4, state.ulButtonPressed));
					}

					if (m_report_state_axis){
						for (int axis = 0; axis < vr::k_unControllerStateAxisCount; axis++){
							//for (int axis = 0; axis < 2; axis++){
							std::ostringstream stringStream;
							stringStream << event_name << +"/State/Axis" << axis << "/";
							std::string axis_name = stringStream.str();
							_dataIndex.addData(axis_name + "Type", getAxisType(unDevice, 0));
							_dataIndex.addData(axis_name + "XPos", state.rAxis[axis].x);
							_dataIndex.addData(axis_name + "YPos", state.rAxis[axis].y);
						}
					}
				}
				_events.push_back(_dataIndex.serialize(event_name));
			}
		}
	}
}

std::string VROpenVRInputDevice::getDeviceName(int idx){
	if (devices[idx] == "")
	{
		//this should never happen. Update deviceNames
		updateDeviceNames();
	}
	
	return devices[idx];
}

void VROpenVRInputDevice::updateDeviceNames()
{
	devices.clear();
	int hmd_count = 0;
	int controller_count = 0;
	int trackingReference_count = 0;
	int unknown_count = 0;
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (m_pHMD->IsTrackedDeviceConnected(unDevice))
		{
			std::string entry;
			vr::TrackedDeviceClass deviceClass = m_pHMD->GetTrackedDeviceClass(unDevice);
			if (deviceClass == vr::TrackedDeviceClass_HMD)
			{
				hmd_count++;
				entry = m_name + "_HMD_" + std::to_string(hmd_count);
			}
			else if (deviceClass == vr::TrackedDeviceClass_Controller){
				controller_count++;
				vr::ETrackedControllerRole role = m_pHMD->GetControllerRoleForTrackedDeviceIndex(unDevice);
				switch (role){
				case vr::TrackedControllerRole_Invalid:
				default:// Invalid value for controller type
					entry = m_name + "_Controller_" + std::to_string(controller_count);
					break;
				case vr::TrackedControllerRole_LeftHand:	
					entry = m_name + "_Controller_Left";
					break;
				case vr::TrackedControllerRole_RightHand:
					entry = m_name + "_Controller_Right";
					break;	
				}
			}
			else if (deviceClass == vr::TrackedDeviceClass_TrackingReference){
				trackingReference_count++;
				entry = m_name + "_TrackingReference_" + std::to_string(trackingReference_count);
			}
			else{
				unknown_count++;
				entry = m_name + "_UnknownDevice_" + std::to_string(unknown_count);
			}
			devices.push_back(entry);
		}
		else
		{
			devices.push_back("");
		}
	}
}

std::string  VROpenVRInputDevice::getButtonName(vr::EVRButtonId id){
	switch( id )
	{
		case vr::k_EButton_System:
			return "SystemButton";
		case vr::k_EButton_ApplicationMenu:
			return "ApplicationMenuButton";
		case vr::k_EButton_Grip:
			return "GripButton";
		case vr::k_EButton_DPad_Left:
			return "DPad_LeftButton";
		case vr::k_EButton_DPad_Up:
			return "DPad_UpButton";
		case vr::k_EButton_DPad_Right:
			return "DPad_RightButton";
		case vr::k_EButton_DPad_Down:
			return "DPad_DownButton";
		case vr::k_EButton_A:
			return "AButton";
		case vr::k_EButton_Axis0:
			return "Axis0Button";
		case vr::k_EButton_Axis1:
			return "Axis1Button";
		case vr::k_EButton_Axis2:
			return "Axis2Button";
		case vr::k_EButton_Axis3:
			return "Axis3Button";
		case vr::k_EButton_Axis4:
			return "Axis4Button";
	}
	return "Invalid";
}

void VROpenVRInputDevice::processVREvent( const vr::VREvent_t & event ,vr::TrackedDevicePose_t *pose)
{	
	
	switch( event.eventType )
	{
		case vr::VREvent_TrackedDeviceActivated:
		{
			updateDeviceNames(); 
			if (m_node->getRenderModelHandler())
				m_node->getRenderModelHandler()->queueModelForLoading(event.trackedDeviceIndex);
			setTipOffset(event.trackedDeviceIndex);
			std::cerr << "Device " << getDeviceName(event.trackedDeviceIndex) << " idx : " << event.trackedDeviceIndex << " attached.\n" << std::endl;		
		}
		break;
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			std::cerr << "Device " << getDeviceName( event.trackedDeviceIndex) << " detached.\n" << std::endl;
			updateDeviceNames();
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			std::cerr << "Device " << getDeviceName(event.trackedDeviceIndex) << " updated.\n" << std::endl;
			updateDeviceNames();
		}
		break;
	case vr::VREvent_ButtonPress :
		{							
			std::string event_name = getDeviceName(event.trackedDeviceIndex) + "_" + getButtonName((vr::EVRButtonId) event.data.controller.button) + "_Pressed";
			//std::cerr << event_name << std::endl;
			_dataIndex.addData(event_name + "/Pose", poseToMatrix4(pose));
			_events.push_back(_dataIndex.serialize(event_name));
		}
		break;
	case vr::VREvent_ButtonUnpress:
		{
			std::string event_name = getDeviceName(event.trackedDeviceIndex) + "_" + getButtonName((vr::EVRButtonId) event.data.controller.button) + "_Released";
			_dataIndex.addData(event_name + "/Pose", poseToMatrix4(pose));
			_events.push_back(_dataIndex.serialize(event_name));
		}
		break;
	case vr::VREvent_ButtonTouch:
		{
			std::string event_name = getDeviceName(event.trackedDeviceIndex) + "_" + getButtonName((vr::EVRButtonId) event.data.controller.button) + "_Touched";
			_dataIndex.addData(event_name + "/Pose", poseToMatrix4(pose));
			_events.push_back(_dataIndex.serialize(event_name));
		}
		break;
	case vr::VREvent_ButtonUntouch:
		{
			std::string event_name = getDeviceName(event.trackedDeviceIndex) + "_" + getButtonName((vr::EVRButtonId) event.data.controller.button) + "_Untouched";
			_dataIndex.addData(event_name + "/Pose", poseToMatrix4(pose));
			_events.push_back(_dataIndex.serialize(event_name));
		}
		break;
	}
}

VRMatrix4 VROpenVRInputDevice::poseToMatrix4(vr::TrackedDevicePose_t *pose)
{
	if(!pose->bPoseIsValid)
		return VRMatrix4();

	
	VRMatrix4 mat = VRMatrix4::fromRowMajorElements(
		pose->mDeviceToAbsoluteTracking.m[0][0], pose->mDeviceToAbsoluteTracking.m[1][0], pose->mDeviceToAbsoluteTracking.m[2][0], 0.0, 
		pose->mDeviceToAbsoluteTracking.m[0][1], pose->mDeviceToAbsoluteTracking.m[1][1], pose->mDeviceToAbsoluteTracking.m[2][1], 0.0,
		pose->mDeviceToAbsoluteTracking.m[0][2], pose->mDeviceToAbsoluteTracking.m[1][2], pose->mDeviceToAbsoluteTracking.m[2][2], 0.0,
		pose->mDeviceToAbsoluteTracking.m[0][3], pose->mDeviceToAbsoluteTracking.m[1][3], pose->mDeviceToAbsoluteTracking.m[2][3], 1.0f
		);

	return mat.transpose();
}

	void VROpenVRInputDevice::setTipOffset(int device)
	{
		if (m_pHMD->IsTrackedDeviceConnected(device) && (m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller))
		{
			char *pchBuffer = new char[10000];
			unsigned int  name_length = 10000;
			m_pHMD->GetStringTrackedDeviceProperty(device, vr::Prop_RenderModelName_String, pchBuffer, name_length, NULL);

			vr::VRControllerState_t c_state;
			if (m_pHMD->GetControllerState(device, &c_state, sizeof(c_state))){
				vr::RenderModel_ControllerMode_State_t statemode;
				statemode.bScrollWheelVisible = false;
				vr::RenderModel_ComponentState_t component_state;

				if (vr::VRRenderModels()->GetComponentState(pchBuffer, "tip", &c_state, &statemode, &component_state))
				{
					m_tip_offset[device] = poseToMatrix4(component_state.mTrackingToComponentLocal.m);
				}

			}
		}
	}

	VRMatrix4 VROpenVRInputDevice::poseToMatrix4(float m[3][4])
{
	VRMatrix4 mat = VRMatrix4::fromRowMajorElements(
		m[0][0], m[1][0], m[2][0], 0.0, 
		m[0][1], m[1][1], m[2][1], 0.0,
		m[0][2], m[1][2], m[2][2], 0.0,
		m[0][3], m[1][3], m[2][3], 1.0f
		);

	return mat.transpose();
}


} /* namespace MinVR */


