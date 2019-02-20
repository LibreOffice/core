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



$(eval $(call gb_Library_Library,AppleRemote))

$(eval $(call gb_Library_add_package_headers,AppleRemote,apple_remote_inc))

$(eval $(call gb_Library_set_include,AppleRemote,\
	$$(INCLUDE) \
	-I$(SRCDIR)/apple_remote/inc \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_linked_libs,AppleRemote,\
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,AppleRemote,\
	-framework Cocoa \
	-framework Carbon \
	-framework IOKit \
))

$(eval $(call gb_Library_add_objcobjects,AppleRemote,\
	apple_remote/source/AppleRemote \
	apple_remote/source/RemoteControl \
	apple_remote/source/RemoteControlContainer \
	apple_remote/source/GlobalKeyboardDevice \
	apple_remote/source/HIDRemoteControlDevice \
	apple_remote/source/MultiClickRemoteBehavior \
	apple_remote/source/RemoteMainController \
	apple_remote/source/KeyspanFrontRowControl \
))

# vim: set noet sw=4 ts=4:

