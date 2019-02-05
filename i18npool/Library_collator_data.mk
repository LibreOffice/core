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



$(eval $(call gb_Library_Library,collator_data))

$(eval $(call gb_Library_set_include,collator_data,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,collator_data, \
#	offapi \
#	udkapi \
#))

#$(eval $(call gb_Library_add_defs,collator_data,\
#	-DI18NPOOL_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_set_versionmap,collator_data,$(SRCDIR)/i18npool/source/collator/data/collator_data.map))

$(eval $(call gb_Library_add_linked_libs,collator_data,\
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_generated_exception_objects,collator_data,\
	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/collator/data/*.txt),CustomTarget/i18npool/source/collator/data/collator_$(notdir $(basename $(txt)))) \
))

$(WORKDIR)/CustomTarget/i18npool/source/collator/data/collator_%.cxx : $(SRCDIR)/i18npool/source/collator/data/%.txt $(OUTDIR)/bin/gencoll_rule$(gb_Executable_EXT)
	mkdir -p $(dir $@) && \
	$(gb_Augment_Library_Path) $(OUTDIR)/bin/gencoll_rule$(gb_Executable_EXT) \
		$(call gb_Helper_convert_native,$<) \
		$(call gb_Helper_convert_native,$@) \
		$*


# vim: set noet sw=4 ts=4:

