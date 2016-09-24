# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,sweb))

$(eval $(call gb_Executable_set_targettype_gui,sweb,YES))

$(eval $(call gb_Executable_add_defs,sweb,\
    -DUNICODE \
))

$(eval $(call gb_Executable_add_ldflags,sweb,\
    /ENTRY:wWinMainCRTStartup \
))

$(eval $(call gb_Executable_use_libraries,sweb,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_static_libraries,sweb,\
    winlauncher \
))

$(eval $(call gb_Executable_add_exception_objects,sweb,\
    desktop/win32/source/applauncher/sweb \
))

$(eval $(call gb_Executable_add_nativeres,sweb,sweb/launcher))

# vim: set ts=4 sw=4 et:
