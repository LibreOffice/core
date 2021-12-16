# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,fpicker))

$(eval $(call gb_Executable_use_api,fpicker,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_set_include,fpicker,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,fpicker,\
    tl \
    sal \
    utl \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

$(eval $(call gb_Executable_add_exception_objects,fpicker,\
    fpicker/source/win32/workbench/Test_fps \
))

# vim: set noet sw=4 ts=4:
