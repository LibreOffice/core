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



$(eval $(call gb_Library_Library,i18npaper))

$(eval $(call gb_Library_add_package_headers,i18npaper,i18npool_inc))

$(eval $(call gb_Library_add_precompiled_header,i18npaper,$(SRCDIR)/i18npool/inc/pch/precompiled_i18npaper))

$(eval $(call gb_Library_set_include,i18npaper,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,i18npaper, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_defs,i18npaper,\
	-DI18NPOOL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,i18npaper,\
	comphelper \
	cppu \
	i18nisolang1 \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,i18npaper,\
	i18npool/source/paper/paper \
))

# vim: set noet sw=4 ts=4:

