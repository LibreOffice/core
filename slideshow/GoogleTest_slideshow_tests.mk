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


$(eval $(call gb_GoogleTest_GoogleTest,slideshow_tests))

$(eval $(call gb_GoogleTest_set_include,slideshow_tests,\
	$$(INCLUDE) \
	-I$(SRCDIR)/slideshow/source/inc \
))

$(eval $(call gb_GoogleTest_add_exception_objects,slideshow_tests, \
	slideshow/test/main \
	slideshow/test/views \
	slideshow/test/slidetest \
	slideshow/test/testshape \
	slideshow/test/testview \
))

$(eval $(call gb_GoogleTest_add_linked_libs,slideshow_tests, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	utl \
	vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_GoogleTest_add_linked_static_libs,slideshow_tests,\
	sldshw_s \
))

$(eval $(call gb_GoogleTest_add_api,slideshow_tests,\
	udkapi \
	offapi \
))

# vim: set noet sw=4 ts=4:
