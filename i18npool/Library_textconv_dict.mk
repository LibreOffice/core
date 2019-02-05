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



$(eval $(call gb_Library_Library,textconv_dict))

$(eval $(call gb_Library_set_include,textconv_dict,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,textconv_dict, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_set_versionmap,textconv_dict,$(SRCDIR)/i18npool/source/textconversion/data/textconv_dict.map))

$(eval $(call gb_Library_add_linked_libs,textconv_dict,\
	stl \
	$(gb_STDLIBS) \
))
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,textconv_dict,\
	cppu \
	cppuhelper \
	sal \
))
else ifeq ($(OS),OS2)
$(eval $(call gb_Library_add_linked_libs,textconv_dict,\
	cppu \
	sal \
))
endif

$(WORKDIR)/CustomTarget/i18npool/source/textconversion/data/%.cxx : $(SRCDIR)/i18npool/source/textconversion/data/%.dic $(OUTDIR)/bin/genconv_dict$(gb_Executable_EXT)
	mkdir -p $(dir $@) && \
	$(gb_Augment_Library_Path) $(OUTDIR)/bin/genconv_dict$(gb_Executable_EXT) $* $(call gb_Helper_convert_native,$<) $(call gb_Helper_convert_native,$@)

$(eval $(call gb_Library_add_generated_exception_objects,textconv_dict,\
	$(foreach dict,$(wildcard $(SRCDIR)/i18npool/source/textconversion/data/*.dic),CustomTarget/i18npool/source/textconversion/data/$(notdir $(basename $(dict)))) \
))

# vim: set noet sw=4 ts=4:

