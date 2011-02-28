#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,directx5canvas))

$(eval $(call gb_Library_set_componentfile,directx5canvas,canvas/source/directx/directx5canvas))

$(eval $(call gb_Library_set_include,directx5canvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
	-I$(SRCDIR)/canvas/source/directx \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,directx5canvas,\
	$$(DEFS) \
	-DDIRECTX_VERSION=0x0500 \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_set_defs,directx5canvas,\
	$$(DEFS) \
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,directx5canvas,\
	cppu \
	tk \
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
