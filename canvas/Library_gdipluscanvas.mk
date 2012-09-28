# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,gdipluscanvas))

$(eval $(call gb_Library_set_componentfile,gdipluscanvas,canvas/source/directx/gdipluscanvas))

$(eval $(call gb_Library_set_include,gdipluscanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_use_sdk_api,gdipluscanvas))

# clear NOMINMAX because GdiplusTypes.h uses those macros :(
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DDIRECTX_VERSION=0x0900 \
	-UNOMINMAX \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_use_libraries,gdipluscanvas,\
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
	i18nisolang1 \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,gdipluscanvas))

$(eval $(call gb_Library_use_system_win32_libs,gdipluscanvas,\
	gdi32 \
	gdiplus \
))

$(eval $(call gb_Library_use_static_libraries,gdipluscanvas,\
	directxcanvas \
))

$(eval $(call gb_Library_add_exception_objects,gdipluscanvas,\
	canvas/source/directx/dx_canvas \
))

# vim: set noet sw=4 ts=4:
