# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,emfio))

$(eval $(call gb_CppunitTest_set_componentfile,emfio,emfio/emfio))

$(eval $(call gb_CppunitTest_set_include,emfio,\
    $$(INCLUDE) \
    -I$(SRCDIR)/emfio/inc \
))

$(eval $(call gb_CppunitTest_use_externals,emfio,\
    boost_headers \
    libxml2 \
    $(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,emfio, \
    emfio/qa/cppunit/wmf/wmfimporttest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,emfio))

$(eval $(call gb_CppunitTest_use_library_objects,emfio, \
    emfio \
))

$(eval $(call gb_CppunitTest_use_libraries,emfio,\
    $(call gb_Helper_optional,BREAKPAD, \
               crashreport) \
    basegfx \
    drawinglayer \
    cppu \
    cppuhelper \
    comphelper \
    i18nlangtag \
    i18nutil \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    sal \
    salhelper \
    sot \
    svl \
    svt \
    test \
    test-setupvcl \
    tl \
    ucbhelper \
    unotest \
    vcl \
    utl \
    xmlreader \
))

$(eval $(call gb_CppunitTest_use_externals,emfio,\
    $(if $(filter LINUX MACOSX %BSD SOLARIS,$(OS)), \
            curl) \
    jpeg \
    $(if $(filter-out IOS WNT,$(OS)), \
            nss3 \
            plc4) \
    libeot \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_add_libs,emfio,\
    -framework IOKit \
    -F/System/Library/PrivateFrameworks \
    -framework CoreUI \
    -lobjc \
))
endif

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,emfio,\
    jvmaccess \
))
endif

$(eval $(call gb_CppunitTest_use_externals,emfio,\
    gio \
    graphite \
    harfbuzz \
    icuuc \
    lcms2 \
))
ifeq ($(ENABLE_HEADLESS),)
$(eval $(call gb_CppunitTest_use_externals,emfio,\
     epoxy \
 ))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,emfio,\
    ApplicationServices \
))
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,emfio,\
    $(if $(filter X86_64,$(CPUNAME)),,QuickTime) \
    Cocoa \
    Carbon \
    CoreFoundation \
))
ifneq ($(ENABLE_MACOSX_SANDBOX),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,emfio,\
    AppleRemote \
))
endif
endif

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_CppunitTest_use_externals,emfio,\
    cairo \
    cups \
    dbus \
    fontconfig \
    freetype \
    valgrind \
))
endif

ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_CppunitTest_use_externals,emfio,\
    cairo \
    freetype \
    fontconfig \
))
else
ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,emfio,\
    -lm \
    -ldl \
    -lpthread \
    -lX11 \
))
endif
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_CppunitTest_add_libs,emfio,\
    -llog \
    -landroid \
    -llo-bootstrap \
))
$(eval $(call gb_CppunitTest_use_externals,emfio,\
    cairo \
    fontconfig \
    freetype \
    expat \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,emfio,\
    UIKit \
    CoreFoundation \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,emfio,\
    advapi32 \
    crypt32 \
    gdi32 \
    gdiplus \
    imm32 \
    mpr \
    ole32 \
    shell32 \
    usp10 \
    uuid \
    version \
    winspool \
    setupapi \
    shlwapi \
))
#$(eval $(call gb_CppunitTest_add_nativeres,emfio,vcl/salsrc))
endif

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_CppunitTest_add_libs,emfio,\
    -lm $(DLOPEN_LIBS) \
    -lpthread \
    -lX11 \
    -lXext \
))
endif

$(eval $(call gb_CppunitTest_use_ure,emfio))

$(eval $(call gb_CppunitTest_use_components,emfio,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,emfio))

# See gb_CppunitTest__use_vcl (solenv/gbuild/CppunitTest.mk; headless):
ifeq ($(USING_X11),TRUE)
$(call gb_CppunitTest_get_target,emfio): \
    $(call gb_Library_get_target,desktop_detector)
endif

# Hack to suppress ASan ODR violation warnings about symbols present in both the
# vcl objects linked into this test library and the vcl library (which gets
# dynamically loaded during the test):
$(call gb_CppunitTest_get_target,emfio): \
    EXTRA_ENV_VARS := \
        ASAN_OPTIONS="$${ASAN_OPTIONS+$$ASAN_OPTIONS:}"detect_odr_violation=0

# vim: set noet sw=4 ts=4:
