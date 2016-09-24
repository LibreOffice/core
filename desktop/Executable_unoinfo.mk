# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,unoinfo))

$(eval $(call gb_Executable_use_static_libraries,unoinfo,\
    ooopathutils \
))

$(eval $(call gb_Executable_add_exception_objects,unoinfo,\
    desktop/win32/source/unoinfo \
))

# vim: set ts=4 sw=4 et:
