# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,desktop_app))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_app, \
    desktop/qa/desktop_app/test_desktop_app \
))

$(eval $(call gb_CppunitTest_use_externals,desktop_app, \
    dbus \
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_app, \
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    editeng \
    i18nlangtag \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    sal \
    salhelper \
    sb \
    sfx \
    svl \
    svxcore \
    svt \
    tk \
    tl \
    utl \
    vcl \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_CppunitTest_use_static_libraries,desktop_app,\
    glxtest \
))

$(eval $(call gb_CppunitTest_add_libs,desktop_app,\
	-lm $(DLOPEN_LIBS) \
	-lpthread \
    -lX11 \
))
endif

$(eval $(call gb_CppunitTest_use_library_objects,desktop_app, \
    sofficeapp \
))

$(eval $(call gb_CppunitTest_use_external,desktop_app,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_app))

# vim: set noet sw=4 ts=4:
