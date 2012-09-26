# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,dbaxml))

$(eval $(call gb_Library_set_include,dbaxml,\
    $$(INCLUDE) \
    -I$(SRCDIR)/dbaccess/source/inc \
))

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
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,dbaxml,dbaccess/source/filter/xml/dbaxml))

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
    dbaccess/source/filter/xml/xmlservices \
    dbaccess/source/filter/xml/xmlStyleImport \
    dbaccess/source/filter/xml/xmlTable \
    dbaccess/source/filter/xml/xmlTableFilterList \
    dbaccess/source/filter/xml/xmlTableFilterPattern \
    dbaccess/source/shared/flt_reghelper \
    dbaccess/source/shared/xmlstrings \
))

# vim: set noet sw=4 ts=4:
