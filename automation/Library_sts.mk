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



$(eval $(call gb_Library_Library,sts))

$(eval $(call gb_Library_add_precompiled_header,sts,$(SRCDIR)/automation/inc/pch/precompiled_automation))

$(eval $(call gb_Library_add_package_headers,sts,automation_inc))

$(eval $(call gb_Library_set_include,sts,\
        $$(INCLUDE) \
	-I$(SRCDIR)/automation/inc \
	-I$(SRCDIR)/automation/inc/pch \
	-I$(SRCDIR)/automation/source/inc \
))

$(eval $(call gb_Library_add_defs,sts,\
	-DAUTOMATION_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,sts,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,sts,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sb \
	sot \
	svl \
	svt \
	tl \
	vcl \
	utl \
	vos3 \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,sts,\
	advapi32 \
	gdi32 \
))
endif

$(eval $(call gb_Library_add_noexception_objects,sts,\
	automation/source/communi/communi \
	automation/source/server/recorder \
	automation/source/server/svcommstream \
	automation/source/server/cmdbasestream \
	automation/source/server/scmdstrm \
	automation/source/server/sta_list \
	automation/source/server/editwin \
	automation/source/server/retstrm \
	automation/source/server/profiler \
	automation/source/simplecm/tcpio \
	automation/source/simplecm/packethandler \
	automation/source/simplecm/simplecm \
))

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_noexception_objects,sts,\
	automation/source/server/prof_usl \
))
else
$(eval $(call gb_Library_add_noexception_objects,sts,\
	automation/source/server/prof_nul \
))
endif

$(eval $(call gb_Library_add_exception_objects,sts,\
	automation/source/server/XMLParser \
	automation/source/server/server \
	automation/source/server/statemnt \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	automation/source/server/statement, $(gb_COMPILERNOOPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
