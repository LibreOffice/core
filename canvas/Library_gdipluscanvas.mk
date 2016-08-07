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



$(eval $(call gb_Library_Library,gdipluscanvas))

$(eval $(call gb_Library_set_componentfile,gdipluscanvas,canvas/source/directx/gdipluscanvas))

$(eval $(call gb_Library_add_api,gdipluscanvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,gdipluscanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DDIRECTX_VERSION=0x0900 \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,gdipluscanvas,\
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
$(eval $(call gb_Library_add_linked_libs,gdipluscanvas,\
	user32 \
	gdi32 \
	gdiplus \
))
endif

$(eval $(call gb_Library_add_exception_objects,gdipluscanvas,\
	canvas/source/directx/dx_bitmap \
	canvas/source/directx/dx_bitmapcanvashelper \
	canvas/source/directx/dx_canvas \
	canvas/source/directx/dx_canvasbitmap \
	canvas/source/directx/dx_canvasfont \
	canvas/source/directx/dx_canvashelper \
	canvas/source/directx/dx_canvashelper_texturefill \
	canvas/source/directx/dx_devicehelper \
	canvas/source/directx/dx_gdiplususer \
	canvas/source/directx/dx_impltools \
	canvas/source/directx/dx_linepolypolygon \
	canvas/source/directx/dx_textlayout \
	canvas/source/directx/dx_textlayout_drawhelper \
	canvas/source/directx/dx_vcltools \
))

# vim: set noet sw=4 ts=4:
