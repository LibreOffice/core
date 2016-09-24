# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,migrationoo2))

$(eval $(call gb_Library_use_external,migrationoo2,boost_headers))

$(eval $(call gb_Library_use_sdk_api,migrationoo2))

$(eval $(call gb_Library_use_libraries,migrationoo2,\
    cppu \
    cppuhelper \
    i18nlangtag \
    $(if $(ENABLE_JAVA), \
        jvmfwk) \
    sal \
    tl \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,migrationoo2,desktop/source/migration/services/migrationoo2))

$(eval $(call gb_Library_add_exception_objects,migrationoo2,\
    desktop/source/migration/services/basicmigration \
    desktop/source/migration/services/cexports \
    $(if $(ENABLE_JAVA), \
        desktop/source/migration/services/jvmfwk) \
    desktop/source/migration/services/wordbookmigration \
))

# vim: set ts=4 sw=4 et:
