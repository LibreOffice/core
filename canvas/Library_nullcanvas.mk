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

$(eval $(call gb_Library_Library,nullcanvas))

$(eval $(call gb_Library_set_componentfile,nullcanvas,canvas/source/null/nullcanvas))

$(eval $(call gb_Library_set_include,nullcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_use_sdk_api,nullcanvas))

$(eval $(call gb_Library_use_libraries,nullcanvas,\
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,nullcanvas))

$(eval $(call gb_Library_add_exception_objects,nullcanvas,\
	canvas/source/null/null_canvasbitmap \
	canvas/source/null/null_canvascustomsprite \
	canvas/source/null/null_canvasfont \
	canvas/source/null/null_canvashelper \
	canvas/source/null/null_devicehelper \
	canvas/source/null/null_spritecanvas \
	canvas/source/null/null_spritecanvashelper \
	canvas/source/null/null_spritehelper \
	canvas/source/null/null_textlayout \
))

# vim: set noet sw=4 ts=4:
