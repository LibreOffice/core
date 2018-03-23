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



$(eval $(call gb_Library_Library,store))

$(eval $(call gb_Library_add_precompiled_header,store,$(SRCDIR)/store/inc/pch/precompiled_store))

$(eval $(call gb_Library_set_versionmap,store,$(SRCDIR)/store/util/store.map))

$(eval $(call gb_Library_set_include,store,\
        $$(INCLUDE) \
	-I$(SRCDIR)/store/inc \
	-I$(SRCDIR)/store/inc/pch \
))

$(eval $(call gb_Library_add_defs,store,\
	-DSTORE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,store,\
	sal \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_noexception_objects,store,\
	store/source/object \
	store/source/lockbyte \
	store/source/storbase \
	store/source/storbios \
	store/source/storcach \
	store/source/stordata \
	store/source/stordir \
	store/source/storlckb \
	store/source/stortree \
	store/source/storpage \
	store/source/store \
))

# vim: set noet sw=4 ts=4:
