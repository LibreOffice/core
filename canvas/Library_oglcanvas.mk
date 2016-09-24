# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,oglcanvas))

$(eval $(call gb_Library_set_componentfile,oglcanvas,canvas/source/opengl/oglcanvas))

$(eval $(call gb_Library_use_sdk_api,oglcanvas))

$(eval $(call gb_Library_use_libraries,oglcanvas,\
	sal \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	tk \
	tl \
	i18nlangtag \
	canvastools \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,oglcanvas,\
	canvas/source/opengl/ogl_bitmapcanvashelper \
	canvas/source/opengl/ogl_canvasbitmap \
	canvas/source/opengl/ogl_canvascustomsprite \
	canvas/source/opengl/ogl_canvasfont \
	canvas/source/opengl/ogl_canvashelper \
	canvas/source/opengl/ogl_canvastools \
	canvas/source/opengl/ogl_spritecanvas \
	canvas/source/opengl/ogl_spritedevicehelper \
	canvas/source/opengl/ogl_textlayout \
	canvas/source/opengl/ogl_texturecache \
))

$(eval $(call gb_Library_use_externals,oglcanvas,\
	boost_headers \
	glew \
))

ifeq ($(strip $(OS)),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,oglcanvas,\
    Cocoa \
    OpenGL \
))

else ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_Library_use_system_win32_libs,oglcanvas,\
    gdi32 \
    glu32 \
    opengl32 \
))

else
$(eval $(call gb_Library_add_libs,oglcanvas,\
    -lGL \
    -lX11 \
))
endif

# vim: set noet sw=4 ts=4:
