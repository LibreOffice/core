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

$(eval $(call gb_Library_Library,dbmm))
$(eval $(call gb_Library_add_package_headers,dbmm,dbaccess_inc))
$(eval $(call gb_Library_add_precompiled_header,dbmm,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbmm))

$(eval $(call gb_Library_set_componentfile,dbmm,dbaccess/source/ext/macromigration/dbmm))

$(eval $(call gb_Library_set_include,dbmm,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/ext/macromigration \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,dbmm,\
	cppu          \
    cppuhelper    \
    comphelper    \
    utl      \
    tl         \
	sal           \
	svt        \
	svl           \
	vcl           \
	svxcore           \
	svx           \
	ucbhelper     \
	stl \
	xcr \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dbmm,\
	dbaccess/source/ext/macromigration/macromigrationwizard \
	dbaccess/source/ext/macromigration/macromigrationdialog \
	dbaccess/source/ext/macromigration/macromigrationpages \
	dbaccess/source/ext/macromigration/dbmm_module \
	dbaccess/source/ext/macromigration/dbmm_services \
	dbaccess/source/ext/macromigration/migrationengine \
	dbaccess/source/ext/macromigration/docinteraction \
	dbaccess/source/ext/macromigration/progresscapture \
	dbaccess/source/ext/macromigration/progressmixer \
	dbaccess/source/ext/macromigration/migrationlog \
	dbaccess/source/ext/macromigration/dbmm_types \
))

# vim: set noet sw=4 ts=4:
