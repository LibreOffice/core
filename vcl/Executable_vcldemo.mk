# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,vcldemo))

$(eval $(call gb_Executable_use_api,vcldemo,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,vcldemo,boost_headers))

$(eval $(call gb_Executable_set_include,vcldemo,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/solenv/inc \
))

$(eval $(call gb_Executable_use_libraries,vcldemo,\
	basegfx \
    tl \
    sal \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

$(eval $(call gb_Executable_add_exception_objects,vcldemo,\
    vcl/workben/vcldemo \
))

$(eval $(call gb_Executable_use_static_libraries,vcldemo,\
    vclmain \
	glxtest \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vcldemo,\
	-lm \
	-ldl \
	-lpthread \
    -lGL \
    -lGLU \
    -lX11 \
))
endif

# vim: set noet sw=4 ts=4:
