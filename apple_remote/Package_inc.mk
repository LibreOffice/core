#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Package_Package,apple_remote_inc,$(SRCDIR)/apple_remote/inc))

$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/AppleRemote.h,AppleRemote.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/GlobalKeyboardDevice.h,GlobalKeyboardDevice.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/HIDRemoteControlDevice.h,HIDRemoteControlDevice.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/KeyspanFrontRowControl.h,KeyspanFrontRowControl.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/MultiClickRemoteBehavior.h,MultiClickRemoteBehavior.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/RemoteControl.h,RemoteControl.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/RemoteControlContainer.h,RemoteControlContainer.h))
$(eval $(call gb_Package_add_file,apple_remote_inc,inc/apple_remote/RemoteMainController.h,RemoteMainController.h))
