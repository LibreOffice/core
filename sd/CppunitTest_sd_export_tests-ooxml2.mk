# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call sd_export_test,-ooxml2))

$(eval $(call gb_CppunitTest_use_uiconfigs,sd_export_tests-ooxml2, \
    modules/simpress \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
