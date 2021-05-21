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

$(eval $(call gb_Module_Module,svx))

$(eval $(call gb_Module_add_targets,svx,\
    Library_svx \
    Library_svxcore \
))

ifeq ($(gb_Side),host)
$(eval $(call gb_Module_add_targets,svx,\
    Library_textconversiondlgs \
    UIConfig_svx \
))
endif

$(eval $(call gb_Module_add_l10n_targets,svx,\
    AllLangMoTarget_svx \
))

$(eval $(call gb_Module_add_check_targets,svx,\
	CppunitTest_svx_unit \
	CppunitTest_svx_gallery_test \
	CppunitTest_svx_removewhichrange \
	CppunitTest_svx_styles \
))

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,svx,\
    CppunitTest_svx_dialogs_test \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
ifeq (,$(DISABLE_DYNLOADING))
$(eval $(call gb_Module_add_targets,svx,\
    Executable_gengal \
    $(if $(filter-out MACOSX WNT,$(OS)), \
		Package_gengal) \
))
endif
else # !DESKTOP
ifeq ($(WITH_GALLERY_BUILD),TRUE)
$(eval $(call gb_Module_add_targets_for_build,svx,\
    Executable_gengal \
))
endif
endif # !DESKTOP

$(eval $(call gb_Module_add_subsequentcheck_targets,svx,\
    JunitTest_svx_unoapi \
))

$(eval $(call gb_Module_add_uicheck_targets,svx,\
    UITest_svx_table \
))

#todo: noopt for EnhanceCustomShapesFunctionParser.cxx on Solaris Sparc and MacOSX
#todo: -DUNICODE and -D_UNICODE on WNT for source/dialog
#todo: component file
# vim: set noet sw=4 ts=4:
