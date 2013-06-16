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
    AllLangResTarget_sw \
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

ifneq ($(DISABLE_SCRIPTING),TRUE)

$(eval $(call gb_Module_add_targets,sw,\
    Library_vbaswobj \
))

endif

$(eval $(call gb_Module_add_check_targets,sw,\
    CppunitTest_sw_uwriter \
))

$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_htmlexport \
    CppunitTest_sw_macros_test \
    CppunitTest_sw_ooxmlexport \
    CppunitTest_sw_ooxmlimport \
    CppunitTest_sw_ww8export \
    CppunitTest_sw_ww8import \
    CppunitTest_sw_rtfexport \
    CppunitTest_sw_rtfimport \
    CppunitTest_sw_odfexport \
    CppunitTest_sw_odfimport \
))

ifneq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_Module_add_slowcheck_targets,sw,\
    CppunitTest_sw_filters_test \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,sw,\
    JunitTest_sw_complex \
    JunitTest_sw_unoapi \
))
endif

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Module_add_subsequentcheck_targets,sw,\
	PythonTest_sw_python \
))
endif

# vim: set noet sw=4 ts=4:
