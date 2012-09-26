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

$(eval $(call gb_Library_Library,canvastools))

$(eval $(call gb_Library_use_package,canvastools,canvas_inc))

$(eval $(call gb_Library_set_include,canvastools,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_use_sdk_api,canvastools))

$(eval $(call gb_Library_add_defs,canvastools,\
	-DCANVASTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,canvastools,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tk \
	tl \
	vcl \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/cachedprimitivebase \
	canvas/source/tools/canvascustomspritehelper \
	canvas/source/tools/canvastools \
	canvas/source/tools/elapsedtime \
	canvas/source/tools/page \
	canvas/source/tools/pagemanager \
	canvas/source/tools/parametricpolypolygon \
	canvas/source/tools/propertysethelper \
	canvas/source/tools/spriteredrawmanager \
	canvas/source/tools/surface \
	canvas/source/tools/surfaceproxy \
	canvas/source/tools/surfaceproxymanager \
	canvas/source/tools/verifyinput \
))

ifeq ($(strip $(ENABLE_AGG)),YES)
$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/bitmap \
	canvas/source/tools/image \
))
$(eval $(call gb_Library_use_libraries,canvastools,\
	agg \
))
ifneq ($(strip $(AGG_VERSION)),)
$(eval $(call gb_Library_add_defs,canvastools,\
	-DAGG_VERSION=$(AGG_VERSION) \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,canvastools,\
	winmm \
))
endif

# vim: set noet sw=4 ts=4:
