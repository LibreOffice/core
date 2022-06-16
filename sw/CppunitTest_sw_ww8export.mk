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

# empty second argument (i.e. no 1)
$(eval $(call sw_ww8export_test,))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ww8export, \
    modules/swriter \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
