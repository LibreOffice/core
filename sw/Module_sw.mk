# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

include $(SRCDIR)/sw/ooxmlexport_setup.mk

$(eval $(call gb_Module_Module,sw))

$(eval $(call gb_Module_add_targets,sw,\
	CustomTarget_generated \
	Library_msword \
	Library_sw \
	Library_swd \
	Library_swui \
	UIConfig_sglobal \
	UIConfig_sweb \
	UIConfig_swform \
	UIConfig_swreport \
	UIConfig_swriter \
	UIConfig_swxform \
))

$(eval $(call gb_Module_add_l10n_targets,sw,\
    AllLangMoTarget_sw \
))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_targets,sw,\
    Library_vbaswobj \
))

endif

ifneq ($(OS),iOS)
$(eval $(call gb_Module_add_check_targets,sw,\
    CppunitTest_sw_uibase_unit \
))

$(eval $(call gb_Module_add_slowcheck_targets,sw,\
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sw_uwriter) \
    CppunitTest_sw_rtfexport \
    CppunitTest_sw_rtfexport2 \
    CppunitTest_sw_rtfexport3 \
    CppunitTest_sw_rtfexport4 \
    CppunitTest_sw_rtfexport5 \
    CppunitTest_sw_docbookexport \
    CppunitTest_sw_fodfexport \
    CppunitTest_sw_htmlexport \
    CppunitTest_sw_xhtmlexport \
    CppunitTest_sw_htmlimport \
    CppunitTest_sw_macros_test \
    CppunitTest_sw_ooxmlexport \
    CppunitTest_sw_ooxmlexport2 \
    CppunitTest_sw_ooxmlexport3 \
    CppunitTest_sw_ooxmlexport4 \
    CppunitTest_sw_ooxmlexport5 \
    CppunitTest_sw_ooxmlexport6 \
    CppunitTest_sw_ooxmlexport7 \
    CppunitTest_sw_ooxmlexport8 \
    CppunitTest_sw_ooxmlexport9 \
    CppunitTest_sw_ooxmlexport10 \
    CppunitTest_sw_ooxmlexport11 \
    CppunitTest_sw_ooxmlexport12 \
    CppunitTest_sw_ooxmlexport13 \
    CppunitTest_sw_ooxmlexport_template \
    CppunitTest_sw_ooxmlfieldexport \
    CppunitTest_sw_ooxmllinks \
    CppunitTest_sw_ooxmlw14export \
    CppunitTest_sw_ooxmlencryption \
    CppunitTest_sw_ooxmlimport \
    CppunitTest_sw_ooxmlimport2 \
    CppunitTest_sw_ww8export \
    CppunitTest_sw_ww8export2 \
    CppunitTest_sw_ww8export3 \
    CppunitTest_sw_ww8import \
    CppunitTest_sw_rtfimport \
    CppunitTest_sw_odfexport \
    CppunitTest_sw_odfimport \
    CppunitTest_sw_txtexport \
    CppunitTest_sw_uiwriter \
    CppunitTest_sw_layoutwriter \
    CppunitTest_sw_mailmerge \
    CppunitTest_sw_globalfilter \
    CppunitTest_sw_accessible_relation_set \
    CppunitTest_sw_apitests \
    CppunitTest_sw_unowriter \
))

ifneq ($(DISABLE_GUI),TRUE)
ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_tiledrendering \
))
endif
endif

ifneq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_filters_test \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,sw,\
    JunitTest_sw_complex \
    JunitTest_sw_unoapi_1 \
    JunitTest_sw_unoapi_2 \
    JunitTest_sw_unoapi_3 \
    JunitTest_sw_unoapi_4 \
))
endif

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Module_add_subsequentcheck_targets,sw,\
	PythonTest_sw_python \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sw,\
    CppunitTest_sw_dialogs_test \
    CppunitTest_sw_dialogs_test_2 \
))

$(eval $(call gb_Module_add_uicheck_targets,sw,\
	UITest_writer_tests \
	UITest_writer_tests2 \
	UITest_writer_tests3 \
	UITest_writer_tests4 \
	UITest_writer_tests5 \
	UITest_table \
	UITest_findReplace \
	UITest_chapterNumbering \
	UITest_librelogo \
	UITest_options \
	UITest_classification \
))
endif

# vim: set noet sw=4 ts=4:
