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



$(eval $(call gb_Library_Library,ctl))

$(eval $(call gb_Library_set_componentfile,ctl,UnoControls/util/ctl))

$(eval $(call gb_Library_set_include,ctl,\
        $$(INCLUDE) \
	-I$(SRCDIR)/UnoControls/inc \
	-I$(SRCDIR)/UnoControls/source/inc \
))

$(eval $(call gb_Library_add_api,ctl,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ctl,\
	cppuhelper \
	cppu \
	sal \
	stl \
	tl \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,ctl,\
	UnoControls/source/base/multiplexer \
	UnoControls/source/base/basecontrol \
	UnoControls/source/base/basecontainercontrol \
	UnoControls/source/base/registercontrols \
	UnoControls/source/controls/progressbar \
	UnoControls/source/controls/framecontrol \
	UnoControls/source/controls/progressmonitor \
	UnoControls/source/controls/OConnectionPointHelper \
	UnoControls/source/controls/OConnectionPointContainerHelper \
	UnoControls/source/controls/statusindicator \
))

# vim: set noet sw=4 ts=4:
