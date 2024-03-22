# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sd/export_setup.mk
include $(SRCDIR)/sd/import_setup.mk

$(eval $(call gb_Module_Module,sd))

$(eval $(call gb_Module_add_targets,sd,\
    Library_sd \
    Library_sdd \
    Library_sdui \
    Package_opengl \
    Package_xml \
    UIConfig_sdraw \
    UIConfig_simpress \
))

$(eval $(call gb_Module_add_l10n_targets,sd,\
    AllLangMoTarget_sd \
))

ifeq ($(filter DRAGONFLY FREEBSD,$(OS)),)
$(eval $(call gb_Module_add_slowcheck_targets,sd,\
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sd_uimpress) \
    CppunitTest_sd_import_tests \
    CppunitTest_sd_import_tests2 \
    CppunitTest_sd_import_tests_skia \
    CppunitTest_sd_import_tests-smartart \
    CppunitTest_sd_export_tests-ooxml1 \
    CppunitTest_sd_export_tests-ooxml2 \
    CppunitTest_sd_export_tests-ooxml3 \
    CppunitTest_sd_export_tests-ooxml4 \
    CppunitTest_sd_export_tests \
    CppunitTest_sd_filters_test \
    CppunitTest_sd_layout_tests \
    CppunitTest_sd_misc_tests \
    CppunitTest_sd_uiimpress \
    CppunitTest_sd_html_export_tests \
    CppunitTest_sd_activex_controls_tests \
    CppunitTest_sd_pdf_import_test \
    CppunitTest_sd_png_export_tests \
    CppunitTest_sd_filter_eppt \
    CppunitTest_sd_shape_import_export_tests \
    CppunitTest_sd_a11y \
    CppunitTest_sd_textfitting_tests \
    CppunitTest_sd_theme_tests \
))
endif

ifneq ($(DISABLE_GUI),TRUE)
ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_slowcheck_targets,sd,\
    CppunitTest_sd_svg_export_tests \
    CppunitTest_sd_tiledrendering \
    CppunitTest_sd_tiledrendering2 \
))
endif
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sd, \
    CppunitTest_sd_dialogs_test \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,sd,\
    JunitTest_sd_unoapi \
))

$(eval $(call gb_Module_add_uicheck_targets,sd,\
	UITest_impress_tests \
	UITest_impress_tests2 \
	UITest_sd_findReplace \
))
# vim: set noet sw=4 ts=4:
