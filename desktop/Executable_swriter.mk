# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,swriter))

$(eval $(call gb_Executable_set_targettype_gui,swriter,YES))

$(eval $(call gb_Executable_add_defs,swriter,\
    -DUNICODE \
))

$(eval $(call gb_Executable_add_ldflags,swriter,\
    /ENTRY:wWinMainCRTStartup \
))

$(eval $(call gb_Executable_use_libraries,swriter,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_static_libraries,swriter,\
    winlauncher \
))

$(eval $(call gb_Executable_add_exception_objects,swriter,\
    desktop/win32/source/applauncher/swriter \
))

$(eval $(call gb_Executable_add_nativeres,swriter,swriter/launcher))

# vim: set ts=4 sw=4 et:
