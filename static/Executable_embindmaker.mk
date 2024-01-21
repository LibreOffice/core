# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,embindmaker))

$(eval $(call gb_Executable_add_exception_objects,embindmaker, \
    static/source/embindmaker/embindmaker \
))

$(eval $(call gb_Executable_use_libraries,embindmaker, \
    sal \
    salhelper \
    unoidl \
))

$(eval $(call gb_Executable_use_static_libraries,embindmaker, \
    codemaker \
))

# vim: set noet sw=4 ts=4:
