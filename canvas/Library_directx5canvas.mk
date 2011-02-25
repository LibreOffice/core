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

$(eval $(call gb_Library_add_exception_objects,directx5canvas,\
	canvas/source/directx/dx_5rm \
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
