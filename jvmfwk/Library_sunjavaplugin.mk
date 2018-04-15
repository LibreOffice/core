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



$(eval $(call gb_Library_Library,sunjavaplugin))

$(eval $(call gb_Library_add_package_headers,sunjavaplugin,jvmfwk_inc))

$(eval $(call gb_Library_add_precompiled_header,sunjavaplugin,$(SRCDIR)/jvmfwk/inc/pch/precompiled_plugin))

$(eval $(call gb_Library_set_versionmap,sunjavaplugin,$(SRCDIR)/jvmfwk/plugins/sunmajor/pluginlib/sunjavaplugin.map))

$(eval $(call gb_Library_set_include,sunjavaplugin,\
	$$(INCLUDE) \
	-I$(SRCDIR)/jvmfwk/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,sunjavaplugin, \
        udkapi \
))

$(eval $(call gb_Library_add_defs,sunjavaplugin,\
        -DJVMFWK_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sunjavaplugin,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,sunjavaplugin,\
	advapi32 \
	uwinapi \
))
endif

$(eval $(call gb_Library_add_exception_objects,sunjavaplugin,\
	jvmfwk/plugins/sunmajor/pluginlib/gnujre \
	jvmfwk/plugins/sunmajor/pluginlib/otherjre \
	jvmfwk/plugins/sunmajor/pluginlib/sunjavaplugin \
	jvmfwk/plugins/sunmajor/pluginlib/sunjre \
	jvmfwk/plugins/sunmajor/pluginlib/sunversion \
	jvmfwk/plugins/sunmajor/pluginlib/util \
	jvmfwk/plugins/sunmajor/pluginlib/vendorbase \
	jvmfwk/plugins/sunmajor/pluginlib/vendorlist \
))

# vim: set noet sw=4 ts=4:

