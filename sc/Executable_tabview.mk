# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,tabview))

$(eval $(call gb_Executable_use_api,tabview,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,tabview,boost_headers))

$(eval $(call gb_Executable_set_include,tabview,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sc/inc \
    -I$(SRCDIR)/sc/source/ui/inc \
))

$(eval $(call gb_Executable_use_libraries,tabview,\
    tl \
    sal \
    sc \
    scui \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

$(eval $(call gb_Executable_add_exception_objects,tabview,\
	sc/workben/tabview \
))

# vim: set noet sw=4 ts=4:
