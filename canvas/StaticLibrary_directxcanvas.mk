# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,directxcanvas))

$(eval $(call gb_StaticLibrary_set_include,directxcanvas,\
    $$(INCLUDE) \
    -I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_StaticLibrary_use_external,directxcanvas,boost_headers))

$(eval $(call gb_StaticLibrary_use_api,directxcanvas,\
    offapi \
    udkapi \
))

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

# vim:set noet sw=4 ts=4:
