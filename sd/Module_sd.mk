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
    Library_sd \
    Library_sdd \
    Library_sdfilt \
    $(call gb_Helper_optional,DESKTOP,Library_sdui) \
    Package_opengl \
    Package_web \
    Package_xml \
))

$(eval $(call gb_Module_add_l10n_targets,sd,\
    AllLangResTarget_sd \
    UIConfig_sdraw \
    UIConfig_simpress \
))

ifeq ($(filter DRAGONFLY FREEBSD,$(OS)),)
$(eval $(call gb_Module_add_check_targets,sd,\
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sd_uimpress) \
    CppunitTest_sd_import_tests \
    CppunitTest_sd_export_ooxml1 \
    CppunitTest_sd_export_ooxml2 \
    CppunitTest_sd_export_tests \
    CppunitTest_sd_filters_test \
    CppunitTest_sd_misc_tests \
    CppunitTest_sd_html_export_tests \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_check_targets,sd,\
    CppunitTest_sd_svg_export_tests \
    CppunitTest_sd_tiledrendering \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sd, \
    CppunitTest_sd_dialogs_test \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,sd,\
    JunitTest_sd_unoapi \
))

# vim: set noet sw=4 ts=4:
