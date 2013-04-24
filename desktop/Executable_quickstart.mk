# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,quickstart))

$(eval $(call gb_Executable_set_targettype_gui,quickstart,YES))

$(eval $(call gb_Executable_use_system_win32_libs,quickstart,\
    comdlg32 \
    gdi32 \
    ole32 \
    oleaut32 \
    shell32 \
))

$(eval $(call gb_Executable_use_libraries,quickstart,\
	$(gb_UWINAPI) \
))

ifeq ($(COM),GCC)

$(eval $(call gb_Executable_use_system_win32_libs,quickstart,\
    uuid \
))

else

$(eval $(call gb_Executable_use_system_win32_libs,quickstart,\
    comsupp \
))

endif

$(eval $(call gb_Executable_add_exception_objects,quickstart,\
    desktop/win32/source/QuickStart/QuickStart \
))

$(eval $(call gb_Executable_add_nativeres,quickstart,quickstart/QuickStart))

# vim: set ts=4 sw=4 et:
