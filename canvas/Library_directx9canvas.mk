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



$(eval $(call gb_Library_Library,directx9canvas))

$(eval $(call gb_Library_set_componentfile,directx9canvas,canvas/source/directx/directx9canvas))

$(eval $(call gb_Library_add_api,directx9canvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,directx9canvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

$(eval $(call gb_Library_add_defs,directx9canvas,\
	-DDIRECTX_VERSION=0x0900 \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,directx9canvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,directx9canvas,\
	cppu \
	ootk \
	sal \
	comphelper \
	cppuhelper \
	stl \
	basegfx \
	canvastools \
	vcl \
	tl \
	utl \
	i18nisolang1 \
    $(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,directx9canvas,\
	user32 \
	gdi32 \
	gdiplus \
))
ifneq ($(USE_MINGW),)
$(eval $(call gb_Library_add_linked_libs,directx9canvas,\
what is $(DIRECTXSDK_LIB)/d3d9.lib now? \
))
else
$(eval $(call gb_Library_add_linked_libs,directx9canvas,\
	d3d9 \
))
endif
endif

$(eval $(call gb_Library_add_exception_objects,directx9canvas,\
	canvas/source/directx/dx_9rm \
	canvas/source/directx/dx_bitmap \
	canvas/source/directx/dx_bitmapcanvashelper \
	canvas/source/directx/dx_canvasbitmap \
	canvas/source/directx/dx_canvascustomsprite \
	canvas/source/directx/dx_canvasfont \
	canvas/source/directx/dx_canvashelper \
	canvas/source/directx/dx_canvashelper_texturefill \
	canvas/source/directx/dx_config \
	canvas/source/directx/dx_devicehelper \
	canvas/source/directx/dx_gdiplususer \
	canvas/source/directx/dx_impltools \
	canvas/source/directx/dx_linepolypolygon \
	canvas/source/directx/dx_spritecanvas \
	canvas/source/directx/dx_spritecanvashelper \
	canvas/source/directx/dx_spritedevicehelper \
	canvas/source/directx/dx_spritehelper \
	canvas/source/directx/dx_surfacebitmap \
	canvas/source/directx/dx_surfacegraphics \
	canvas/source/directx/dx_textlayout \
	canvas/source/directx/dx_textlayout_drawhelper \
	canvas/source/directx/dx_vcltools \
))

# vim: set noet sw=4 ts=4:
