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

$(eval $(call gb_Library_Library,directx9canvas))

$(eval $(call gb_Library_set_include,directx9canvas,\
    $$(INCLUDE) \
    -I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_set_componentfile,directx9canvas,canvas/source/directx/directx9canvas,services))

$(eval $(call gb_Library_use_sdk_api,directx9canvas))

$(eval $(call gb_Library_use_external,directx9canvas,boost_headers))

$(eval $(call gb_Library_use_libraries,directx9canvas,\
	cppu \
	tk \
	sal \
	comphelper \
	cppuhelper \
	basegfx \
	canvastools \
	vcl \
	tl \
	utl \
	i18nlangtag \
))

$(eval $(call gb_Library_use_system_win32_libs,directx9canvas,\
	d3d9 \
	gdi32 \
	gdiplus \
))

$(eval $(call gb_Library_use_static_libraries,directx9canvas,\
	directxcanvas \
))

$(eval $(call gb_Library_add_exception_objects,directx9canvas,\
	canvas/source/directx/dx_9rm \
	canvas/source/directx/dx_canvascustomsprite \
	canvas/source/directx/dx_config \
	canvas/source/directx/dx_spritecanvas \
	canvas/source/directx/dx_spritecanvashelper \
	canvas/source/directx/dx_spritedevicehelper \
	canvas/source/directx/dx_spritehelper \
	canvas/source/directx/dx_surfacebitmap \
	canvas/source/directx/dx_surfacegraphics \
))

# vim: set noet sw=4 ts=4:
