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
	$(if $(ENABLE_BREAKPAD),breakpad) \
    dbus \
    icu_headers \
    icui18n \
    icuuc \
	$(if $(ENABLE_ONLINE_UPDATE_MAR),\
		curl \
        orcus-parser \
        orcus )\
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_app, \
    comphelper \
    cppu \
    cppuhelper \
    $(if $(ENABLE_BREAKPAD),crashreport) \
    deploymentmisc \
    editeng \
    i18nlangtag \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    sal \
    salhelper \
    sb \
    sfx \
    svl \
    svx \
    svxcore \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_static_libraries,desktop_app,\
    $(if $(ENABLE_ONLINE_UPDATE_MAR),\
        windows_process )\
))
endif


ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
ifeq ($(USING_X11),TRUE)
$(eval $(call gb_CppunitTest_use_static_libraries,desktop_app,\
    glxtest \
))
endif

$(eval $(call gb_CppunitTest_add_libs,desktop_app,\
	-lm $(DLOPEN_LIBS) \
    -lX11 \
))
endif

$(eval $(call gb_CppunitTest_use_library_objects,desktop_app, \
    sofficeapp \
))

ifeq ($(ENABLE_MACOSX_SANDBOX),TRUE)

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,desktop_app,\
    Foundation \
))

endif

$(eval $(call gb_CppunitTest_use_external,desktop_app,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_app))

# vim: set noet sw=4 ts=4:
