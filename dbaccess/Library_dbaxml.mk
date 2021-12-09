# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbaxml))

$(eval $(call gb_Library_set_include,dbaxml,\
    $$(INCLUDE) \
    -I$(SRCDIR)/dbaccess/inc \
    -I$(SRCDIR)/dbaccess/source/inc \
))

$(eval $(call gb_Library_use_external,dbaxml,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,dbaxml,dbaccess/inc/pch/precompiled_dbaxml))

$(eval $(call gb_Library_use_sdk_api,dbaxml))

$(eval $(call gb_Library_use_libraries,dbaxml,\
    comphelper \
    cppu \
    cppuhelper \
    dba \
    dbtools \
    sal \
    salhelper \
    sax \
    sfx \
    svl \
    tk \
    tl \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_Library_set_componentfile,dbaxml,dbaccess/source/filter/xml/dbaxml,services))

$(eval $(call gb_Library_add_exception_objects,dbaxml,\
    dbaccess/source/filter/xml/dbloader2 \
    dbaccess/source/filter/xml/xmlAutoStyle \
    dbaccess/source/filter/xml/xmlColumn \
    dbaccess/source/filter/xml/xmlComponent \
    dbaccess/source/filter/xml/xmlConnectionData \
    dbaccess/source/filter/xml/xmlConnectionResource \
    dbaccess/source/filter/xml/xmlDatabase \
    dbaccess/source/filter/xml/xmlDatabaseDescription \
    dbaccess/source/filter/xml/xmlDataSource \
    dbaccess/source/filter/xml/xmlDataSourceInfo \
    dbaccess/source/filter/xml/xmlDataSourceSetting \
    dbaccess/source/filter/xml/xmlDataSourceSettings \
    dbaccess/source/filter/xml/xmlDocuments \
    dbaccess/source/filter/xml/xmlExport \
    dbaccess/source/filter/xml/xmlFileBasedDatabase \
    dbaccess/source/filter/xml/xmlfilter \
    dbaccess/source/filter/xml/xmlHelper \
    dbaccess/source/filter/xml/xmlHierarchyCollection \
    dbaccess/source/filter/xml/xmlLogin \
    dbaccess/source/filter/xml/xmlQuery \
    dbaccess/source/filter/xml/xmlServerDatabase \
    dbaccess/source/filter/xml/xmlStyleImport \
    dbaccess/source/filter/xml/xmlTable \
    dbaccess/source/filter/xml/xmlTableFilterList \
    dbaccess/source/filter/xml/xmlTableFilterPattern \
))

# vim: set noet sw=4 ts=4:
