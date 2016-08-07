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



$(eval $(call gb_Library_Library,cppcanvas))

$(eval $(call gb_Library_add_package_headers,cppcanvas,cppcanvas_inc))

$(eval $(call gb_Library_add_precompiled_header,cppcanvas,$(SRCDIR)/cppcanvas/inc/pch/precompiled_cppcanvas))

$(eval $(call gb_Library_add_api,cppcanvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,cppcanvas,\
	-I$(SRCDIR)/cppcanvas/inc \
	-I$(SRCDIR)/cppcanvas/inc/pch \
	-I$(SRCDIR)/cppcanvas/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,cppcanvas,\
	-DCPPCANVAS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,cppcanvas,\
	tl \
	cppu \
	sal \
	vcl \
	stl \
	comphelper \
	canvastools \
	cppuhelper \
	basegfx \
	i18nisolang1 \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,cppcanvas,\
	cppcanvas/source/mtfrenderer/bitmapaction \
	cppcanvas/source/mtfrenderer/cachedprimitivebase \
	cppcanvas/source/mtfrenderer/implrenderer \
	cppcanvas/source/mtfrenderer/lineaction \
	cppcanvas/source/mtfrenderer/mtftools \
	cppcanvas/source/mtfrenderer/pointaction \
	cppcanvas/source/mtfrenderer/polypolyaction \
	cppcanvas/source/mtfrenderer/textaction \
	cppcanvas/source/mtfrenderer/transparencygroupaction \
	cppcanvas/source/tools/canvasgraphichelper \
	cppcanvas/source/tools/tools \
	cppcanvas/source/wrapper/basegfxfactory \
	cppcanvas/source/wrapper/implbitmap \
	cppcanvas/source/wrapper/implbitmapcanvas \
	cppcanvas/source/wrapper/implcanvas \
	cppcanvas/source/wrapper/implcolor \
	cppcanvas/source/wrapper/implcustomsprite \
	cppcanvas/source/wrapper/implfont \
	cppcanvas/source/wrapper/implpolypolygon \
	cppcanvas/source/wrapper/implsprite \
	cppcanvas/source/wrapper/implspritecanvas \
	cppcanvas/source/wrapper/impltext \
	cppcanvas/source/wrapper/vclfactory \
))

# vim: set noet sw=4 ts=4:
