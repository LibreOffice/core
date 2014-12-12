# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mtfdemo))

$(eval $(call gb_Executable_use_api,mtfdemo,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,mtfdemo,boost_headers))

$(eval $(call gb_Executable_set_include,mtfdemo,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/solenv/inc \
))

$(eval $(call gb_Executable_use_libraries,mtfdemo,\
	basegfx \
    tl \
    sal \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

$(eval $(call gb_Executable_add_exception_objects,mtfdemo,\
    vcl/workben/mtfdemo \
))

$(eval $(call gb_Executable_use_static_libraries,mtfdemo,\
    vclmain \
))

ifeq ($(OS),LINUX)
ifeq ($(ENABLE_OPENGL_CANVAS),TRUE)
$(eval $(call gb_Executable_add_libs,mtfdemo,\
	-lm \
	-ldl \
	-lpthread \
    -lGL \
    -lGLU \
    -lX11 \
))

$(eval $(call gb_Executable_use_static_libraries,mtfdemo,\
	glxtest \
))
endif
endif

# vim: set noet sw=4 ts=4:
