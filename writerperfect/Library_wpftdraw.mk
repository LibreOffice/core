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

$(eval $(call gb_Library_Library,wpftdraw))

$(eval $(call gb_Library_set_componentfile,wpftdraw,writerperfect/util/wpftdraw))

$(eval $(call gb_Library_set_include,wpftdraw,\
	$$(INCLUDE) \
    -I$(SRCDIR)/writerperfect/source \
))

$(eval $(call gb_Library_use_api,wpftdraw,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,wpftdraw,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sot \
	tl \
	utl \
	xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_static_libraries,wpftdraw,\
	writerperfect \
))

$(eval $(call gb_Library_use_externals,wpftdraw,\
	wpd \
	wpg \
	visio \
	cdr \
	lcms2 \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,wpftdraw,\
	writerperfect/source/draw/CDRImportFilter \
	writerperfect/source/draw/CMXImportFilter \
	writerperfect/source/draw/VisioImportFilter \
	writerperfect/source/draw/WPGImportFilter \
	writerperfect/source/draw/wpftdraw_genericfilter \
))

# vim: set noet sw=4 ts=4:
