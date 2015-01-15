# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,textdemo))

$(eval $(call gb_Executable_use_api,textdemo,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,textdemo,boost_headers))

$(eval $(call gb_Executable_set_include,textdemo,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/solenv/inc \
))

$(eval $(call gb_Executable_use_libraries,textdemo,\
	basegfx \
    tl \
    sal \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
    svt \
))

$(eval $(call gb_Executable_add_exception_objects,textdemo,\
    vcl/workben/textdemo \
))

$(eval $(call gb_Executable_use_static_libraries,textdemo,\
    vclmain \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,textdemo,\
	-lm \
	-ldl \
	-lpthread \
    -lGL \
    -lGLU \
    -lX11 \
))

$(eval $(call gb_Executable_use_static_libraries,textdemo,\
	glxtest \
))
endif

# vim: set noet sw=4 ts=4:
