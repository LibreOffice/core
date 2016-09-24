# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Rdb_Rdb,uno_services))

$(eval $(call gb_Rdb_add_components,uno_services,\
    testtools/source/bridgetest/cppobj \
    testtools/source/bridgetest/constructors \
    testtools/source/bridgetest/bridgetest \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Rdb_add_components,uno_services,\
    testtools/source/bridgetest/testComponent \
))
endif

# vim:set noet sw=4 ts=4:
