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



$(eval $(call gb_Library_Library,dict_zh))

$(eval $(call gb_Library_set_include,dict_zh,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,dict_zh, \
#        udkapi \
#	offapi \
#))

$(eval $(call gb_Library_set_versionmap,dict_zh,$(SRCDIR)/i18npool/source/breakiterator/data/dict.map))

$(eval $(call gb_Library_add_linked_libs,dict_zh,\
	$(gb_STDLIBS) \
))

$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/dict_%.cxx : $(SRCDIR)/i18npool/source/breakiterator/data/%.dic $(OUTDIR)/bin/gendict$(gb_Executable_EXT)
	mkdir -p $(dir $@) && \
	$(gb_Augment_Library_Path) $(OUTDIR)/bin/gendict$(gb_Executable_EXT) $(call gb_Helper_convert_native,$<) $(call gb_Helper_convert_native,$@)

$(eval $(call gb_Library_add_generated_exception_objects,dict_zh,\
	CustomTarget/i18npool/source/breakiterator/data/dict_zh \
))

# vim: set noet sw=4 ts=4:

