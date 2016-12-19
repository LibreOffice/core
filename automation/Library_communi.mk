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



$(eval $(call gb_Library_Library,communi))

$(eval $(call gb_Library_add_precompiled_header,communi,$(SRCDIR)/automation/inc/pch/precompiled_automation))

$(eval $(call gb_Library_add_package_headers,communi,automation_inc))

$(eval $(call gb_Library_set_include,communi,\
        $$(INCLUDE) \
	-I$(SRCDIR)/automation/inc \
	-I$(SRCDIR)/automation/inc/pch \
	-I$(SRCDIR)/automation/source/inc \
))

$(eval $(call gb_Library_add_defs,communi,\
	-DAUTOMATION_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,communi,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,communi,\
	sal \
	simplecm \
	svl \
	tl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,communi,\
	advapi32 \
	gdi32 \
))
endif

$(eval $(call gb_Library_add_noexception_objects,communi,\
	automation/source/communi/communi \
))


# vim: set noet sw=4 ts=4:
