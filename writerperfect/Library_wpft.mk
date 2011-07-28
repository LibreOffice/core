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

$(eval $(call gb_Library_Library,wpft))

$(eval $(call gb_Library_set_componentfile,wpft,writerperfect/util/wpft))

$(eval $(call gb_Library_set_include,wpft,\
	$$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerperfect/source) \
))

$(eval $(call gb_Library_add_api,wpft,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,wpft,\
	cppu \
	cppuhelper \
	vcl \
	sal \
	sfx \
	sot \
	tl \
	ucbhelper \
	utl \
	xo \
))

$(eval $(call gb_Library_add_linked_static_libs,wpft,\
	writerperfect \
))

$(eval $(call gb_Library_use_externals,wpft,\
	wpd \
	wpg \
))

$(eval $(call gb_Library_add_exception_objects,wpft,\
	writerperfect/source/wpdimp/WordPerfectImportFilter \
	writerperfect/source/wpdimp/wpft_genericfilter \
))

# vim: set noet sw=4:
