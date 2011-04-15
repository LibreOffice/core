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

$(eval $(call gb_Library_Library,dbaxml))
$(eval $(call gb_Library_add_package_headers,dbaxml,dbaccess_inc))
$(eval $(call gb_Library_add_precompiled_header,dbaxml,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbaxml))

$(eval $(call gb_Library_set_componentfile,dbaxml,dbaccess/source/filter/xml/dbaxml))

$(eval $(call gb_Library_set_include,dbaxml,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/filter/xml \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,dbaxml,\
	tl \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	utl \
	vcl \
	sal \
	sfx \
	dbtools \
	xo \
	sot \
	tk \
	dba \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dbaxml,\
	dbaccess/source/filter/xml/xmlfilter                           \
	dbaccess/source/filter/xml/dbloader2                           \
	dbaccess/source/filter/xml/xmlDatabase                         \
	dbaccess/source/filter/xml/xmlDataSource                       \
	dbaccess/source/filter/xml/xmlTableFilterPattern       \
	dbaccess/source/filter/xml/xmlTableFilterList          \
	dbaccess/source/filter/xml/xmlDataSourceInfo           \
	dbaccess/source/filter/xml/xmlDataSourceSetting        \
	dbaccess/source/filter/xml/xmlDataSourceSettings       \
	dbaccess/source/filter/xml/xmlDocuments                        \
	dbaccess/source/filter/xml/xmlComponent                        \
	dbaccess/source/filter/xml/xmlHierarchyCollection      \
	dbaccess/source/filter/xml/xmlLogin                            \
	dbaccess/source/filter/xml/xmlExport                           \
	dbaccess/source/filter/xml/xmlQuery                            \
	dbaccess/source/filter/xml/xmlTable                            \
	dbaccess/source/filter/xml/xmlColumn                           \
	dbaccess/source/filter/xml/xmlStyleImport                      \
	dbaccess/source/filter/xml/xmlHelper                           \
	dbaccess/source/filter/xml/xmlAutoStyle                        \
	dbaccess/source/filter/xml/xmlConnectionData           \
	dbaccess/source/filter/xml/xmlDatabaseDescription      \
	dbaccess/source/filter/xml/xmlFileBasedDatabase        \
	dbaccess/source/filter/xml/xmlServerDatabase           \
	dbaccess/source/filter/xml/xmlConnectionResource       \
	dbaccess/source/filter/xml/xmlservices \
))

$(eval $(call gb_Library_add_noexception_objects,dbaxml, \
	dbaccess/source/shared/flt_reghelper \
	dbaccess/source/shared/xmlstrings \
))

# vim: set noet sw=4 ts=4:
