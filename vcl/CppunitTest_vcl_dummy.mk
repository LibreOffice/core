# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_dummy))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_dummy, \
    vcl/qa/cppunit/dummy \
))

$(eval $(call gb_CppunitTest_use_library_objects,vcl_dummy,vcl))


$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	$(if $(filter LINUX MACOSX %BSD SOLARIS,$(OS)), \
		curl) \
	jpeg \
	$(if $(filter-out IOS WNT,$(OS)), \
		nss3 \
		plc4) \
	libeot \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_dummy,\
    $(call gb_Helper_optional,BREAKPAD, \
		crashreport) \
    svl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    cppu \
    sal \
    salhelper \
    xmlreader \
	$(gb_UWINAPI) \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_add_libs,vcl_dummy,\
    -framework IOKit \
    -F/System/Library/PrivateFrameworks \
    -framework CoreUI \
    -lobjc \
))
endif

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,vcl_dummy,\
    jvmaccess \
))
endif

$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	gio \
	harfbuzz \
	icuuc \
	lcms2 \
))
ifeq ($(ENABLE_HEADLESS),)
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
     glew \
 ))
endif

ifeq ($(ENABLE_GRAPHITE),TRUE)
$(eval $(call gb_CppunitTest_use_external,vcl_dummy,graphite))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,vcl_dummy,\
	ApplicationServices \
))
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,vcl_dummy,\
    $(if $(filter X86_64,$(CPUNAME)),,QuickTime) \
    Cocoa \
    Carbon \
    CoreFoundation \
	OpenGL \
))
ifneq ($(ENABLE_MACOSX_SANDBOX),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,vcl_dummy,\
    AppleRemote \
))
endif
endif

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	cairo \
	cups \
	dbus \
	fontconfig \
	freetype \
	valgrind \
))
endif

ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	cairo \
	freetype \
))
ifneq ($(OS),EMSCRIPTEN)
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	fontconfig \
))
endif
else
ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,vcl_dummy,\
	-lm \
	-ldl \
	-lpthread \
    -lGL \
    -lX11 \
))
endif
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_CppunitTest_add_libs,vcl_dummy,\
	-llog \
	-landroid \
	-llo-bootstrap \
))
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	cairo \
	fontconfig \
	freetype \
	expat \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,vcl_dummy,\
	UIKit \
	CoreFoundation \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,vcl_dummy,\
	advapi32 \
	crypt32 \
	gdi32 \
	gdiplus \
    glu32 \
	imm32 \
	mpr \
	msimg32 \
    opengl32 \
	ole32 \
	shell32 \
	usp10 \
	uuid \
	version \
	winspool \
	setupapi \
	shlwapi \
))
#$(eval $(call gb_CppunitTest_add_nativeres,vcl_dummy,vcl/salsrc))
endif

ifeq ($(OS), WNT)
$(eval $(call gb_CppunitTest_use_externals,vcl_dummy,\
	glyphy \
))
endif

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_CppunitTest_add_libs,vcl_dummy,\
	-lm $(DLOPEN_LIBS) \
	-lpthread \
    -lGL \
    -lX11 \
	-lXext \
))
endif

# vim: set noet sw=4 ts=4:
