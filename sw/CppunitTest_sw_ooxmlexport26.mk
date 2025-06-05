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

$(eval $(call sw_ooxmlexport_test,26))

# this test requires "de" translations and if missing tests will be skipped
$(call gb_CppunitTest_get_target,sw_ooxmlexport26) : $(call gb_AllLangMoTarget_get_target,sw)

$(call gb_CppunitTest_get_target,sw_ooxmlexport26) : export LO_TEST_LOCALE=de

# vim: set noet sw=4 ts=4:
