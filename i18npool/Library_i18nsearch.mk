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



$(eval $(call gb_Library_Library,i18nsearch))

$(eval $(call gb_Library_add_package_headers,i18nsearch,i18npool_inc))

$(eval $(call gb_Library_add_precompiled_header,i18nsearch,$(SRCDIR)/i18npool/inc/pch/precompiled_search))

$(eval $(call gb_Library_set_componentfile,i18nsearch,i18npool/source/search/i18nsearch))

$(eval $(call gb_Library_set_include,i18nsearch,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,i18nsearch, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_defs,i18nsearch,\
	-DI18NPOOL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,i18nsearch,\
	cppu \
	cppuhelper \
	i18nisolang1 \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_external,i18nsearch,icui18n)
$(call gb_Library_use_external,i18nsearch,icuuc)


$(eval $(call gb_Library_add_exception_objects,i18nsearch,\
	i18npool/source/search/levdis \
	i18npool/source/search/textsearch \
))

# vim: set noet sw=4 ts=4:

