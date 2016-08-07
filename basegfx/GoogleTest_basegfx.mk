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


$(eval $(call gb_GoogleTest_GoogleTest,basegfx_test))

$(eval $(call gb_GoogleTest_add_exception_objects,basegfx_test, \
	basegfx/test/basegfx1d \
	basegfx/test/basegfx2d \
	basegfx/test/basegfx3d \
	basegfx/test/boxclipper \
	basegfx/test/basegfxtools \
	basegfx/test/clipstate \
	basegfx/test/genericclipper \
	basegfx/test/main \
	basegfx/test/testtools \
))

$(eval $(call gb_GoogleTest_add_linked_libs,basegfx_test, \
	basegfx \
	sal \
	stl \
	cppuhelper \
	cppu \
    $(gb_STDLIBS) \
))

$(eval $(call gb_GoogleTest_add_api,basegfx_test,\
	udkapi \
	offapi \
))

$(eval $(call gb_GoogleTest_set_include,basegfx_test,\
	$$(INCLUDE) \
	-I$(SRCDIR)/basegfx/inc/pch \
))

# vim: set noet sw=4 ts=4:
