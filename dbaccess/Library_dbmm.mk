# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,dbmm))

$(eval $(call gb_Library_add_package_headers,dbmm,\
	dbaccess_inc \
))

$(eval $(call gb_Library_add_precompiled_header,dbmm,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbaccess))

$(eval $(call gb_Library_set_include,dbmm,\
    $$(INCLUDE) \
	-I$(realpath $(SRCDIR)/dbaccess/inc) \
    -I$(realpath $(SRCDIR)/dbaccess/inc/pch) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,dbmm,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,dbmm,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svl \
    svt \
    svxcore \
    svx \
    tl \
    ucbhelper \
    utl \
    vcl \
    xcr \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,dbmm,dbaccess/source/ext/macromigration/dbmm))

$(eval $(call gb_Library_add_exception_objects,dbmm,\
    dbaccess/source/ext/macromigration/dbmm_module \
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
