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



$(eval $(call gb_Library_Library,jvmfwk))

$(eval $(call gb_Library_add_package_headers,jvmfwk,jvmfwk_inc))

$(eval $(call gb_Library_add_precompiled_header,jvmfwk,$(SRCDIR)/jvmfwk/inc/pch/precompiled_jvmfwk))

$(eval $(call gb_Library_set_versionmap,jvmfwk,$(SRCDIR)/jvmfwk/source/framework.map))

$(eval $(call gb_Library_set_include,jvmfwk,\
	$$(INCLUDE) \
	-I$(SRCDIR)/jvmfwk/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,jvmfwk, \
        udkapi \
))

$(eval $(call gb_Library_add_defs,jvmfwk,\
        -DJVMFWK_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,jvmfwk,\
	cppuhelper \
	sal \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,jvmfwk,\
	advapi32 \
))
endif

$(eval $(call gb_Library_use_externals,jvmfwk,\
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,jvmfwk,\
	jvmfwk/source/elements \
	jvmfwk/source/framework \
	jvmfwk/source/fwkbase \
	jvmfwk/source/fwkutil \
	jvmfwk/source/libxmlutil \
))

# vim: set noet sw=4 ts=4:

