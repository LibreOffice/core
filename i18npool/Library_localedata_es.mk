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



$(eval $(call gb_Library_Library,localedata_es))

$(eval $(call gb_Library_set_include,localedata_es,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,localedata_es, \
#	offapi \
#	udkapi \
#))

#$(eval $(call gb_Library_add_defs,localedata_es,\
#	-DI18NPOOL_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_set_versionmap,localedata_es,$(SRCDIR)/i18npool/source/localedata/data/localedata_es.map))

$(eval $(call gb_Library_add_linked_libs,localedata_es,\
	localedata_en \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_es,\
	CustomTarget/i18npool/source/localedata/data/localedata_es_AR \
	CustomTarget/i18npool/source/localedata/data/localedata_es_BO \
	CustomTarget/i18npool/source/localedata/data/localedata_es_CL \
	CustomTarget/i18npool/source/localedata/data/localedata_es_CO \
	CustomTarget/i18npool/source/localedata/data/localedata_es_CR \
	CustomTarget/i18npool/source/localedata/data/localedata_es_DO \
	CustomTarget/i18npool/source/localedata/data/localedata_es_EC \
	CustomTarget/i18npool/source/localedata/data/localedata_es_ES \
	CustomTarget/i18npool/source/localedata/data/localedata_es_GT \
	CustomTarget/i18npool/source/localedata/data/localedata_es_HN \
	CustomTarget/i18npool/source/localedata/data/localedata_es_MX \
	CustomTarget/i18npool/source/localedata/data/localedata_es_NI \
	CustomTarget/i18npool/source/localedata/data/localedata_es_PA \
	CustomTarget/i18npool/source/localedata/data/localedata_es_PE \
	CustomTarget/i18npool/source/localedata/data/localedata_es_PR \
	CustomTarget/i18npool/source/localedata/data/localedata_es_PY \
	CustomTarget/i18npool/source/localedata/data/localedata_es_SV \
	CustomTarget/i18npool/source/localedata/data/localedata_es_UY \
	CustomTarget/i18npool/source/localedata/data/localedata_es_VE \
	CustomTarget/i18npool/source/localedata/data/localedata_gl_ES \
))


# vim: set noet sw=4 ts=4:

