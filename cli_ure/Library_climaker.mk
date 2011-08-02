# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,climaker))

$(eval $(call gb_Library_use_udk_api,climaker))

$(eval $(call gb_Library_set_visibility_default,climaker))

$(eval $(call gb_Library_use_libraries,climaker,\
        sal \
        cppu \
        cppuhelper \
))

$(eval $(call gb_Library_add_exception_objects,climaker,\
        cli_ure/source/climaker/climaker_mono \
))

# vim: set noet sw=4 ts=4:
