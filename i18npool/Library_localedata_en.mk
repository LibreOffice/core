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



$(eval $(call gb_Library_Library,localedata_en))

$(eval $(call gb_Library_set_include,localedata_en,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,localedata_en, \
#	offapi \
#	udkapi \
#))

#$(eval $(call gb_Library_add_defs,localedata_en,\
#	-DI18NPOOL_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_set_versionmap,localedata_en,$(SRCDIR)/i18npool/source/localedata/data/localedata_en.map))

$(eval $(call gb_Library_add_linked_libs,localedata_en,\
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_en,\
	CustomTarget/i18npool/source/localedata/data/localedata_en_AU \
	CustomTarget/i18npool/source/localedata/data/localedata_en_BZ \
	CustomTarget/i18npool/source/localedata/data/localedata_en_CA \
	CustomTarget/i18npool/source/localedata/data/localedata_en_GB \
	CustomTarget/i18npool/source/localedata/data/localedata_en_GH \
	CustomTarget/i18npool/source/localedata/data/localedata_en_IE \
	CustomTarget/i18npool/source/localedata/data/localedata_en_JM \
	CustomTarget/i18npool/source/localedata/data/localedata_en_NA \
	CustomTarget/i18npool/source/localedata/data/localedata_en_NZ \
	CustomTarget/i18npool/source/localedata/data/localedata_en_PH \
	CustomTarget/i18npool/source/localedata/data/localedata_en_TT \
	CustomTarget/i18npool/source/localedata/data/localedata_en_US \
	CustomTarget/i18npool/source/localedata/data/localedata_en_ZA \
	CustomTarget/i18npool/source/localedata/data/localedata_en_ZW \
))

ifeq ($(OS),WNT)
URL_PROTOCOL=file:///
else
URL_PROTOCOL=file://
endif

$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/localedata_%.cxx : $(SRCDIR)/i18npool/source/localedata/data/%.xml $(OUTDIR)/bin/saxparser$(gb_Executable_EXT) $(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.rdb
	$(gb_Augment_Library_Path) $(OUTDIR)/bin/saxparser$(gb_Executable_EXT) \
		$* \
		$(call gb_Helper_convert_native,$<) \
		$(call gb_Helper_convert_native,$@) \
		$(URL_PROTOCOL)$(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.rdb) \
		$(call gb_Helper_convert_native,$(OUTDIR)/bin/types.rdb) \
		-env:OOO_INBUILD_SHAREDLIB_DIR=$(URL_PROTOCOL)$(call gb_Helper_convert_native,$(gb_InBuild_Library_Path))

MY_COMPONENTS := component/sax/source/expatwrap/expwrap

$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.rdb : \
			$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.input \
			$(SOLARENV)/bin/packcomponents.xslt \
			$(foreach component,$(MY_COMPONENTS),$(OUTDIR)/xml/$(component).inbuild.component)
	$(gb_XSLTPROC) \
		--nonet \
		--stringparam prefix $(call gb_Helper_convert_native,$(OUTDIR)/xml/) \
		-o $(call gb_Helper_convert_native,$@) \
		$(call gb_Helper_convert_native,$(SOLARENV)/bin/packcomponents.xslt) \
		$(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.input)

$(WORKDIR)/CustomTarget/i18npool/source/localedata/data/saxparser.input :
	mkdir -p $(dir $@) \
	&& echo "<list>$(foreach component,$(MY_COMPONENTS),<filename>$(component).inbuild.component</filename>)</list>" > $@

# vim: set noet sw=4 ts=4:

