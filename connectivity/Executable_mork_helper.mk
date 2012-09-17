# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mork_helper))

$(eval $(call gb_Executable_set_include,mork_helper,\
    -I$(SRCDIR)/connectivity/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_sdk_api,mork_helper))

$(eval $(call gb_Executable_use_libraries,mork_helper,\
    cppu \
    cppuhelper \
    mork \
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,mork_helper,\
    connectivity/source/drivers/mork/mork_helper \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
