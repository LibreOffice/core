# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,senddoc))

$(eval $(call gb_Executable_use_system_win32_libs,senddoc,\
	kernel32 \
))

$(eval $(call gb_Executable_use_libraries,senddoc,\
	sal \
))

$(eval $(call gb_Executable_use_static_libraries,senddoc,\
	simplemapi \
))

$(eval $(call gb_Executable_add_exception_objects,senddoc,\
    shell/source/win32/simplemail/senddoc \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
