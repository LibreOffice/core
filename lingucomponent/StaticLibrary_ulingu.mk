###############################################################
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
###############################################################



$(eval $(call gb_StaticLibrary_StaticLibrary,ulingu))

$(eval $(call gb_StaticLibrary_set_include,ulingu,\
        $$(INCLUDE) \
        -I$(SRCDIR)/lingucomponent/inc \
        -I$(SRCDIR)/lingucomponent/inc/pch \
        -I$(OUTDIR)/inc \
))

$(eval $(call gb_StaticLibrary_add_api,ulingu,\
	udkapi \
	offapi \
))

ifeq ($(SYSTEM_DICTS),YES)
$(eval $(call gb_StaticLibrary_add_defs,ulingu,\
	-DSYSTEM_DICTS \
	-DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" \
	-DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" \
	-DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\" \
))
endif

$(eval $(call gb_StaticLibrary_add_exception_objects,ulingu,\
	lingucomponent/source/lingutil/lingutil \
))

# vim: set noet sw=4 ts=4:
