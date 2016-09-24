# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,simpress))

$(eval $(call gb_Executable_set_targettype_gui,simpress,YES))

$(eval $(call gb_Executable_add_defs,simpress,\
    -DUNICODE \
))

$(eval $(call gb_Executable_add_ldflags,simpress,\
    /ENTRY:wWinMainCRTStartup \
))

$(eval $(call gb_Executable_use_libraries,simpress,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_static_libraries,simpress,\
    winlauncher \
))

$(eval $(call gb_Executable_add_exception_objects,simpress,\
    desktop/win32/source/applauncher/simpress \
))

$(eval $(call gb_Executable_add_nativeres,simpress,simpress/launcher))

# vim: set ts=4 sw=4 et:
