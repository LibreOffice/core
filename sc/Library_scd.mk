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



$(eval $(call gb_Library_Library,scd))

$(eval $(call gb_Library_add_precompiled_header,scd,$(SRCDIR)/sc/inc/pch/precompiled_scd))

$(eval $(call gb_Library_set_componentfile,scd,sc/util/scd))

$(eval $(call gb_Library_set_include,scd,\
        $$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/inc/pch \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
))

$(eval $(call gb_Library_add_defs,scd,\
	-DSC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,scd,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,scd,\
	cppu \
	cppuhelper \
	sal \
	sfx \
	sot \
	stl \
	svl \
	svt \
	tl \
	ucbhelper \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,scd,\
	sc/source/ui/unoobj/scdetect \
	sc/source/ui/unoobj/detreg \
))

# vim: set noet sw=4 ts=4:
