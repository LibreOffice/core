# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,visualbackendtest))

$(eval $(call gb_Executable_use_api,visualbackendtest,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,visualbackendtest,boost_headers))

$(eval $(call gb_Executable_set_include,visualbackendtest,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,visualbackendtest,\
	basegfx \
    comphelper \
    cppu \
    cppuhelper \
    tl \
    sal \
	salhelper \
    vcl \
))

$(eval $(call gb_Executable_add_exception_objects,visualbackendtest,\
    vcl/backendtest/VisualBackendTest \
))

$(eval $(call gb_Executable_use_static_libraries,visualbackendtest,\
    vclmain \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,visualbackendtest,\
	-lm \
	-ldl \
    -lX11 \
))

$(eval $(call gb_Executable_use_static_libraries,visualbackendtest,\
	glxtest \
))
endif

# vim: set noet sw=4 ts=4:
