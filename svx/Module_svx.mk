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
    Library_textconversiondlgs \
))

$(eval $(call gb_Module_add_l10n_targets,svx,\
    AllLangResTarget_svx \
    AllLangResTarget_ofa \
    AllLangResTarget_gal \
    UIConfig_svx \
))

$(eval $(call gb_Module_add_check_targets,svx,\
	CppunitTest_svx_unit \
))

$(eval $(call gb_Module_add_slowcheck_targets,svx,\
    CppunitTest_svx_dialogs_test \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,svx,\
    Executable_gengal \
    $(if $(filter-out MACOSX WNT,$(OS)), \
		Package_gengal) \
    $(if $(filter-out WNT,$(OS)), \
        Executable_pixelctl) \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,svx,\
    JunitTest_svx_unoapi \
))
endif

#todo: noopt for EnhanceCustomShapesFunctionParser.cxx on Solaris Sparc and MacOSX
#todo: -DBOOST_SPIRIT_USE_OLD_NAMESPACE only in CustomShapes ?
#todo: -DUNICODE and -D_UNICODE on WNT for source/dialog
#todo: component file
# vim: set noet sw=4 ts=4:
