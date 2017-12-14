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


$(eval $(call gb_Library_Library,slideshow))

$(eval $(call gb_Library_set_componentfile,slideshow,slideshow/util/slideshow))

$(eval $(call gb_Library_add_linked_libs,slideshow,\
	avmedia \
	basegfx \
	canvastools \
	comphelper \
	cppcanvas \
	cppuhelper \
	cppu \
	sal \
	stl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_static_libs,slideshow,\
	sldshw_s \
))

# List this file again, even though it's in the static lib, so that
# component_getFactory and component_getImplementationEnvironment are exported:
$(eval $(call gb_Library_add_exception_objects,slideshow,\
	slideshow/source/engine/slideshowimpl \
))

# vim: set noet sw=4 ts=4:
