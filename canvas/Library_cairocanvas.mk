# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,cairocanvas))

$(eval $(call gb_Library_set_componentfile,cairocanvas,canvas/source/cairo/cairocanvas))

$(eval $(call gb_Library_use_sdk_api,cairocanvas))

ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_add_cxxflags,cairocanvas,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,cairocanvas,\
    Cocoa \
))

endif

ifeq ($(OS),IOS)

$(eval $(call gb_Library_add_cxxflags,cairocanvas,\
    $(gb_OBJCXXFLAGS) \
))

endif

$(eval $(call gb_Library_use_libraries,cairocanvas,\
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

$(eval $(call gb_Library_use_externals,cairocanvas,\
	boost_headers \
	cairo \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_win32_cairo \
))
$(eval $(call gb_Library_use_libraries,cairocanvas,\
	gdi32 \
))

else

ifneq (,$(filter MACOSX IOS,$(OS)))
$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_quartz_cairo \
))
else ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_xlib_cairo \
))

# freetype? fontconfig? -> test on Solaris
$(eval $(call gb_Library_add_libs,cairocanvas,\
	-lX11 \
))
$(eval $(call gb_Library_use_externals,cairocanvas,\
	Xrender \
))
endif

endif

# vim: set noet sw=4 ts=4:
