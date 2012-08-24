# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# [ Copyright (C) 2012 Red Hat, Inc., Michael Stahl <mstahl@redhat.com> (initial developer) ]
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_StaticLibrary_StaticLibrary,directxcanvas))

$(eval $(call gb_StaticLibrary_set_include,directxcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_StaticLibrary_use_api,directxcanvas,\
    offapi \
    udkapi \
))

# clear NOMINMAX because GdiplusTypes.h uses those macros :(
$(eval $(call gb_StaticLibrary_add_defs,directxcanvas,\
	-DDIRECTX_VERSION=0x0900 \
	-UNOMINMAX \
))

ifeq ($(WINDOWS_SDK_VERSION),80)
$(eval $(call gb_StaticLibrary_add_defs,directxcanvas,\
	-DWIN8_SDK=1 \
))
endif

$(eval $(call gb_StaticLibrary_add_exception_objects,directxcanvas,\
	canvas/source/directx/dx_bitmap \
	canvas/source/directx/dx_bitmapcanvashelper \
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
