# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_com))

$(eval $(call gb_Executable_set_targettype_gui,soffice_com,NO))

$(eval $(call gb_Executable_use_system_win32_libs,soffice_com,\
    shell32 \
))

$(eval $(call gb_Executable_use_static_libraries,soffice_com,\
    ooopathutils \
    winloader \
))

$(eval $(call gb_Executable_add_exception_objects,soffice_com,\
    desktop/win32/source/officeloader/soffice_com \
))

$(eval $(call gb_Executable_add_nativeres,soffice_com,soffice/launcher))

# vim: set ts=4 sw=4 et:
