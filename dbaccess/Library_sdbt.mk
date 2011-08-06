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

$(eval $(call gb_Library_Library,sdbt))

$(eval $(call gb_Library_add_package_headers,sdbt,\
	dbaccess_inc \
))

$(eval $(call gb_Library_add_precompiled_header,sdbt,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbaccess))

$(eval $(call gb_Library_set_include,sdbt,\
    $$(INCLUDE) \
	-I$(realpath $(SRCDIR)/dbaccess/inc) \
	-I$(realpath $(SRCDIR)/dbaccess/inc/pch) \
	-I$(realpath $(SRCDIR)/dbaccess/source/inc/) \
	-I$(realpath $(SRCDIR)/dbaccess/source/sdbtools/inc) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,sdbt,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,sdbt,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    sal \
    tl \
    utl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,sdbt,dbaccess/util/sdbt))

$(eval $(call gb_Library_add_exception_objects,sdbt,\
    dbaccess/source/sdbtools/connection/connectiontools \
    dbaccess/source/sdbtools/connection/datasourcemetadata \
    dbaccess/source/sdbtools/connection/objectnames \
    dbaccess/source/sdbtools/connection/tablename \
    dbaccess/source/sdbtools/misc/module_sdbt \
    dbaccess/source/sdbtools/misc/sdbt_services \
))

$(eval $(call gb_Library_add_noexception_objects,sdbt,\
    dbaccess/source/shared/sdbtstrings \
))

# vim: set noet sw=4 ts=4:
