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

$(eval $(call gb_Library_Library,adabasui))
$(eval $(call gb_Library_add_package_headers,adabasui,dbaccess_inc))

$(eval $(call gb_Library_set_componentfile,adabasui,dbaccess/source/ext/adabas/adabasui))

$(eval $(call gb_Library_set_include,adabasui,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/ext/adabas \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_defs,adabasui,\
	-DCOMPMOD_NAMESPACE=adabasui \
))

$(eval $(call gb_Library_add_linked_libs,adabasui,\
	tl \
	svt \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	utl \
	ucbhelper \
	vcl \
	sal \
	stl \
	sfx \
	tk \
	dbtools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,adabasui,\
    dbaccess/source/ext/adabas/ASqlNameEdit \
	dbaccess/source/ext/adabas/AdabasNewDb \
	dbaccess/source/ext/adabas/ANewDb \
	dbaccess/source/ext/adabas/Aservices \
	dbaccess/source/ext/adabas/adabasuistrings \
	dbaccess/source/ext/adabas/Acomponentmodule \
))

# vim: set noet sw=4 ts=4:
