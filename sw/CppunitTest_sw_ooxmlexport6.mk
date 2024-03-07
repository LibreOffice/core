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

$(eval $(call sw_ooxmlexport_test,6))

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_ooxmlexport6,abort))
endif

# vim: set noet sw=4 ts=4:
