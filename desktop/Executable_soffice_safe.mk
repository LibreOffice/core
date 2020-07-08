# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_safe))

$(eval $(call gb_Executable_set_targettype_gui,soffice_safe,YES))

$(eval $(call gb_Executable_add_ldflags,soffice_safe,\
    /ENTRY:wWinMainCRTStartup \
))

$(eval $(call gb_Executable_use_static_libraries,soffice_safe,\
    winlauncher \
))

$(eval $(call gb_Executable_add_exception_objects,soffice_safe,\
    desktop/win32/source/applauncher/soffice_safe \
))

$(eval $(call gb_Executable_add_nativeres,soffice_safe,soffice/launcher))

$(eval $(call gb_Executable_add_default_nativeres,soffice_safe,$(PRODUCTNAME)))

# vim: set ts=4 sw=4 et:
