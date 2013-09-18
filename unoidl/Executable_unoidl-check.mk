# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,unoidl-check))

$(eval $(call gb_Executable_add_exception_objects,unoidl-check, \
    unoidl/source/unoidl-check \
))

$(eval $(call gb_Executable_use_libraries,unoidl-check, \
    sal \
    salhelper \
    unoidl \
))

# vim: set noet sw=4 ts=4:
