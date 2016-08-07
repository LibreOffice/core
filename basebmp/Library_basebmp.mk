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



$(eval $(call gb_Library_Library,basebmp))

$(eval $(call gb_Library_add_package_headers,basebmp,basebmp_inc))

$(eval $(call gb_Library_add_api,basebmp,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,basebmp,\
	-I$(SRCDIR)/basebmp/inc/ \
	-I$(SRCDIR)/basebmp/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,basebmp,\
	-DBASEBMP_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,basebmp,\
    sal \
    basegfx \
    stl \
    $(gb_STDLIBS) \
))

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_cxxobjects,basebmp, \
	basebmp/source/bitmapdevice \
	basebmp/source/debug \
	basebmp/source/polypolygonrenderer \
    , $(gb_COMPILEROPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) -xalias_level=compatible \
))
else
$(eval $(call gb_Library_add_exception_objects,basebmp,\
	basebmp/source/bitmapdevice \
	basebmp/source/debug \
	basebmp/source/polypolygonrenderer \
))
endif

# vim: set noet sw=4 ts=4:
