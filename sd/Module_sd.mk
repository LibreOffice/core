# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sd))

$(eval $(call gb_Module_add_targets,sd,\
    AllLangResTarget_sd \
    Library_sd \
    Library_sdd \
    Library_sdfilt \
    Library_sdui \
    Package_inc \
    Package_web \
    Package_xml \
    UIConfig_sdraw \
    UIConfig_simpress \
))

ifneq ($(OS),DRAGONFLY)
$(eval $(call gb_Module_add_check_targets,sd,\
    CppunitTest_sd_uimpress \
    CppunitTest_sd_regression_test \
    CppunitTest_sd_filters_test \
))
endif


$(eval $(call gb_Module_add_subsequentcheck_targets,sd,\
    JunitTest_sd_unoapi \
))

# vim: set noet sw=4 ts=4:
