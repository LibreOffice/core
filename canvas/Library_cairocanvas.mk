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



$(eval $(call gb_Library_Library,cairocanvas))

$(eval $(call gb_Library_set_componentfile,cairocanvas,canvas/source/cairo/cairocanvas))

$(eval $(call gb_Library_add_api,cairocanvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,cairocanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,cairocanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	sal \
	stl \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	ootk \
	tl \
	i18nisolang1 \
	canvastools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_cachedbitmap \
	canvas/source/cairo/cairo_cairo \
	canvas/source/cairo/cairo_canvas \
	canvas/source/cairo/cairo_canvasbitmap \
	canvas/source/cairo/cairo_canvascustomsprite \
	canvas/source/cairo/cairo_canvasfont \
	canvas/source/cairo/cairo_canvashelper \
	canvas/source/cairo/cairo_canvashelper_text \
	canvas/source/cairo/cairo_devicehelper \
	canvas/source/cairo/cairo_services \
	canvas/source/cairo/cairo_spritecanvas \
	canvas/source/cairo/cairo_spritecanvashelper \
	canvas/source/cairo/cairo_spritedevicehelper \
	canvas/source/cairo/cairo_spritehelper \
	canvas/source/cairo/cairo_textlayout \
))

$(call gb_Library_use_external,cairocanvas,cairo)

ifeq ($(OS),WNT)

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_win32_cairo \
))
$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	gdi32 \
	user32 \
))

else

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_quartz_cairo \
))

#CFLAGSCXX+=$(OBJCXXFLAGS)

else

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_xlib_cairo \
))

# freetype? fontconfig? -> test on Solaris
$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	X11 \
	Xrender \
))

endif

endif

# vim: set noet sw=4 ts=4:
