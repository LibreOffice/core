# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_StaticLibrary_StaticLibrary,writerperfect))

$(eval $(call gb_StaticLibrary_set_include,writerperfect,\
	$$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerperfect/source) \
))

$(eval $(call gb_StaticLibrary_add_api,writerperfect,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,writerperfect,\
	writerperfect/source/filter/DocumentElement \
	writerperfect/source/filter/DocumentHandler \
	writerperfect/source/filter/FontStyle \
	writerperfect/source/filter/InternalHandler \
	writerperfect/source/filter/ListStyle \
	writerperfect/source/filter/OdgGenerator \
	writerperfect/source/filter/OdtGenerator \
	writerperfect/source/filter/PageSpan \
	writerperfect/source/filter/SectionStyle \
	writerperfect/source/filter/TableStyle \
	writerperfect/source/filter/TextRunStyle \
	writerperfect/source/stream/WPXSvStream \
))

# vim: set noet sw=4 ts=4:
