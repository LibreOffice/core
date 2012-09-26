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

$(eval $(call gb_Library_Library,vclcanvas))

$(eval $(call gb_Library_set_componentfile,vclcanvas,canvas/source/vcl/vclcanvas))

$(eval $(call gb_Library_set_include,vclcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_use_sdk_api,vclcanvas))

$(eval $(call gb_Library_use_libraries,vclcanvas,\
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	sal \
	svt \
	tk \
	tl \
	vcl \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/backbuffer \
	canvas/source/vcl/bitmapbackbuffer \
	canvas/source/vcl/cachedbitmap \
	canvas/source/vcl/canvas \
	canvas/source/vcl/canvasbitmap \
	canvas/source/vcl/canvasbitmaphelper \
	canvas/source/vcl/canvascustomsprite \
	canvas/source/vcl/canvasfont \
	canvas/source/vcl/canvashelper \
	canvas/source/vcl/devicehelper \
	canvas/source/vcl/impltools \
	canvas/source/vcl/services \
	canvas/source/vcl/spritecanvas \
	canvas/source/vcl/spritecanvashelper \
	canvas/source/vcl/spritedevicehelper \
	canvas/source/vcl/spritehelper \
	canvas/source/vcl/textlayout \
	canvas/source/vcl/canvashelper_texturefill \
	canvas/source/vcl/windowoutdevholder \
))

# vim: set noet sw=4 ts=4:
