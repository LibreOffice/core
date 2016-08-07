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



$(eval $(call gb_Library_Library,directx5canvas))

$(eval $(call gb_Library_set_componentfile,directx5canvas,canvas/source/directx/directx5canvas))

$(eval $(call gb_Library_add_api,directx5canvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,directx5canvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
	-I$(SRCDIR)/canvas/source/directx \
))

$(eval $(call gb_Library_add_defs,directx5canvas,\
	-DDIRECTX_VERSION=0x0500 \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,directx5canvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,directx5canvas,\
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
$(eval $(call gb_Library_add_linked_libs,directx5canvas,\
	user32 \
	gdi32 \
	gdiplus \
	ddraw \
))
ifneq ($(USE_MINGW),)
$(eval $(call gb_Library_add_linked_libs,directx5canvas,\
what is $(DIRECTXSDK_LIB)/d3dx.lib now? \
))
else
$(eval $(call gb_Library_add_linked_libs,directx5canvas,\
	d3dx \
))
endif
endif

$(WORKDIR)/CustomTarget/canvas/source/directx/%.cxx : $(SRCDIR)/canvas/source/directx/%.cxx
	mkdir -p $(dir $@) && \
	cp $< $@

$(eval $(call gb_Library_add_generated_exception_objects,directx5canvas,\
	CustomTarget/canvas/source/directx/dx_5rm \
	CustomTarget/canvas/source/directx/dx_bitmap \
	CustomTarget/canvas/source/directx/dx_bitmapcanvashelper \
	CustomTarget/canvas/source/directx/dx_canvasbitmap \
	CustomTarget/canvas/source/directx/dx_canvascustomsprite \
	CustomTarget/canvas/source/directx/dx_canvasfont \
	CustomTarget/canvas/source/directx/dx_canvashelper \
	CustomTarget/canvas/source/directx/dx_canvashelper_texturefill \
	CustomTarget/canvas/source/directx/dx_config \
	CustomTarget/canvas/source/directx/dx_devicehelper \
	CustomTarget/canvas/source/directx/dx_gdiplususer \
	CustomTarget/canvas/source/directx/dx_impltools \
	CustomTarget/canvas/source/directx/dx_linepolypolygon \
	CustomTarget/canvas/source/directx/dx_spritecanvas \
	CustomTarget/canvas/source/directx/dx_spritecanvashelper \
	CustomTarget/canvas/source/directx/dx_spritedevicehelper \
	CustomTarget/canvas/source/directx/dx_spritehelper \
	CustomTarget/canvas/source/directx/dx_surfacebitmap \
	CustomTarget/canvas/source/directx/dx_surfacegraphics \
	CustomTarget/canvas/source/directx/dx_textlayout \
	CustomTarget/canvas/source/directx/dx_textlayout_drawhelper \
	CustomTarget/canvas/source/directx/dx_vcltools \
))

# vim: set noet sw=4 ts=4:
