# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,chart2))

$(eval $(call gb_Module_add_targets,chart2,\
    Library_chartcontroller \
    Library_chartcore \
))
ifeq ($(ENABLE_OPENGL),TRUE)
$(eval $(call gb_Module_add_targets,chart2,\
     Library_chartopengl \
     Package_opengl \
 ))
endif

$(eval $(call gb_Module_add_l10n_targets,chart2,\
    AllLangResTarget_chartcontroller \
	UIConfig_chart2 \
))

$(eval $(call gb_Module_add_check_targets,chart2,\
	CppunitTest_chart2_common_functors \
))

$(eval $(call gb_Module_add_slowcheck_targets,chart2,\
    CppunitTest_chart2_export \
    CppunitTest_chart2_import \
    CppunitTest_chart2_trendcalculators \
))

ifeq ($(ENABLE_CHART_TESTS),TRUE)
$(eval $(call gb_Module_add_slowcheck_targets,chart2,\
    CppunitTest_chart2_xshape \
))

endif

$(eval $(call gb_Module_add_subsequentcheck_targets,chart2,\
    JunitTest_chart2_unoapi \
))

$(eval $(call gb_Module_add_slowcheck_targets,chart2,\
    CppunitTest_chart2_dialogs_test \
))

# vim: set noet sw=4 ts=4:
