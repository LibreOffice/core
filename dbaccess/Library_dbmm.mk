# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbmm))

$(eval $(call gb_Library_set_include,dbmm,\
    $$(INCLUDE) \
	-I$(SRCDIR)/dbaccess/inc \
))

$(eval $(call gb_Library_use_external,dbmm,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,dbmm,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbmm))

$(eval $(call gb_Library_use_sdk_api,dbmm))

$(eval $(call gb_Library_use_libraries,dbmm,\
    comphelper \
    cppu \
    cppuhelper \
    dba \
    sal \
    svl \
    svt \
    svxcore \
    svx \
    tl \
    ucbhelper \
    utl \
    vcl \
    xmlscript \
))

$(eval $(call gb_Library_set_componentfile,dbmm,dbaccess/source/ext/macromigration/dbmm))

$(eval $(call gb_Library_add_exception_objects,dbmm,\
    dbaccess/source/ext/macromigration/dbmm_services \
    dbaccess/source/ext/macromigration/dbmm_types \
    dbaccess/source/ext/macromigration/docinteraction \
    dbaccess/source/ext/macromigration/macromigrationdialog \
    dbaccess/source/ext/macromigration/macromigrationpages \
    dbaccess/source/ext/macromigration/macromigrationwizard \
    dbaccess/source/ext/macromigration/migrationengine \
    dbaccess/source/ext/macromigration/migrationlog \
    dbaccess/source/ext/macromigration/progresscapture \
    dbaccess/source/ext/macromigration/progressmixer \
))

# vim: set noet sw=4 ts=4:
