# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,benchmark))

$(eval $(call gb_Executable_use_api,benchmark,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,benchmark,boost_headers))

$(eval $(call gb_Executable_set_include,benchmark,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,benchmark,\
	basegfx \
    comphelper \
    cppu \
    cppuhelper \
    tl \
    sal \
	salhelper \
))

$(eval $(call gb_Executable_use_vclmain,benchmark,-lm))

$(eval $(call gb_Executable_add_exception_objects,benchmark,\
    vcl/backendtest/benchmark/BenchmarkApp \
))

# vim: set noet sw=4 ts=4:
