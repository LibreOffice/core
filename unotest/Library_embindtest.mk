# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,embindtest))

$(eval $(call gb_Library_add_exception_objects,embindtest, \
    unotest/source/embindtest/embindtest \
))

$(eval $(call gb_Library_set_componentfile,embindtest,unotest/source/embindtest/embindtest,services))

$(eval $(call gb_Library_use_sdk_api,embindtest))

# vim: set noet sw=4 ts=4:
