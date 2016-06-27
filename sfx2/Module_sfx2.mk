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

$(eval $(call gb_Module_Module,sfx2))

$(eval $(call gb_Module_add_targets,sfx2,\
    CustomTarget_classification \
    Library_sfx \
    Package_classification \
    Package_emoji \
))

$(eval $(call gb_Module_add_l10n_targets,sfx2,\
    AllLangResTarget_sfx2 \
    UIConfig_sfx \
))

$(eval $(call gb_Module_add_check_targets,sfx2,\
    CppunitTest_sfx2_metadatable \
    CppunitTest_sfx2_controlleritem \
    CppunitTest_sfx2_classification \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_subsequentcheck_targets,sfx2,\
    JunitTest_sfx2_complex \
    JunitTest_sfx2_unoapi \
))
endif

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Module_add_subsequentcheck_targets,sfx2,\
	PythonTest_sfx2_python \
))
endif

ifneq (,$(filter LINUX DRAGONFLY OPENBSD FREEBSD NETBSD SOLARIS, $(OS)))
ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Module_add_targets,sfx2,\
    Library_qstart_gtk \
))
endif
endif

#todo: clean up quickstarter stuff in both libraries
#todo: move standard pool to svl

# vim: set noet sw=4 ts=4:
