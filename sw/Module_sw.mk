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

$(eval $(call gb_Module_Module,sw))

$(eval $(call gb_Module_add_targets,sw,\
        CustomTarget_generated \
	Library_msword \
	Library_sw \
	Library_swd \
	$(call gb_Helper_optional,DESKTOP,Library_swui) \
))

$(eval $(call gb_Module_add_l10n_targets,sw,\
    AllLangResTarget_sw \
	UIConfig_sglobal \
	UIConfig_sweb \
	UIConfig_swform \
	UIConfig_swreport \
	UIConfig_swriter \
	UIConfig_swxform \
))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_targets,sw,\
    Library_vbaswobj \
))

endif

$(eval $(call gb_Module_add_slowcheck_targets,sw,\
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sw_uwriter) \
    CppunitTest_sw_docbookexport \
    CppunitTest_sw_htmlexport \
    CppunitTest_sw_htmlimport \
    CppunitTest_sw_macros_test \
    CppunitTest_sw_ooxmlexport \
    CppunitTest_sw_ooxmlexport2 \
    CppunitTest_sw_ooxmlexport3 \
    CppunitTest_sw_ooxmlexport4 \
    CppunitTest_sw_ooxmlexport5 \
    CppunitTest_sw_ooxmlexport6 \
    CppunitTest_sw_ooxmlexport7 \
    CppunitTest_sw_ooxmlfieldexport \
    CppunitTest_sw_ooxmlw14export \
    CppunitTest_sw_ooxmlimport \
    CppunitTest_sw_ww8export \
    CppunitTest_sw_ww8import \
    CppunitTest_sw_rtfexport \
    CppunitTest_sw_rtfimport \
    CppunitTest_sw_odfexport \
    CppunitTest_sw_odfimport \
    CppunitTest_sw_uiwriter \
    CppunitTest_sw_mailmerge \
    CppunitTest_sw_globalfilter \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_tiledrendering \
))
endif

ifneq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_filters_test \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sw,\
    CppunitTest_sw_dialogs_test \
))

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

# vim: set noet sw=4 ts=4:
