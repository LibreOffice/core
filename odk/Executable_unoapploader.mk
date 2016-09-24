# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,unoapploader))

$(eval $(call gb_Executable_use_static_libraries,unoapploader,\
	findsofficepath \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_set_targettype_gui,unoapploader,YES))

$(eval $(call gb_Executable_add_cobjects,unoapploader,\
	odk/source/unoapploader/win/unoapploader \
))

$(eval $(call gb_Executable_use_system_win32_libs,unoapploader,\
	advapi32 \
))
else
$(eval $(call gb_Executable_add_cobjects,unoapploader,\
	odk/source/unoapploader/unx/unoapploader \
))

$(eval $(call gb_Executable_add_libs,unoapploader,\
    $(if $(filter-out FREEBSD NETBSD DRAGONFLY,$(OS)),-ldl) \
))
endif

# vim: set ts=4 sw=4 et:
