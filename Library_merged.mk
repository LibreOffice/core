# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Matúš Kukan <matus.kukan@gmail.com> (C) 2011, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,merged))

# gb_MERGEDLIBS is defined in solenv/gbuild/extensions/pre_MergedLibsList.mk
$(eval $(call gb_Library_use_library_objects,merged,\
	$(gb_MERGEDLIBS) \
))

$(eval $(call gb_Library_use_libraries,merged,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
    jvmfwk \
	sal \
	salhelper \
	tl \
	ucbhelper \
	xmlreader \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,merged,\
	cups \
	icule \
	icuuc \
	jpeg \
	lcms2 \
	libxml2 \
	nss3 \
	zlib \
))

# This would be needed on all platforms but only on android we merge libraries from lingucomponent
ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_use_externals,merged,\
	hunspell \
	expat_utf8 \
))
$(eval $(call gb_Library_use_static_libraries,merged,\
	sax_shared \
	ulingu \
))
$(eval $(call gb_Library_add_libs,merged,\
	$(if $(filter MSC,$(COM)),libmythes.lib,-lmythes-1.2) \
	$(if $(filter MSC,$(COM)),hyphen.lib,-lhyphen) \
))
endif

ifeq ($(ENABLE_GRAPHITE),TRUE)
$(eval $(call gb_Library_use_externals,merged,\
	graphite \
))
endif

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_externals,merged,\
	berkeleydb \
))
endif

ifneq ($(OS),IOS)
$(eval $(call gb_Library_use_libraries,merged,\
	jvmaccess \
))
endif

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_use_externals,merged,\
	fontconfig \
	freetype \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,merged,\
	-lm \
	-ldl \
	-lpthread \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,merged,\
	advapi32 \
	gdi32 \
	gdiplus \
	imm32 \
	mpr \
	msimg32 \
	oldnames \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	usp10 \
	uuid \
	version \
	winmm \
	winspool \
))
$(eval $(call gb_Library_use_externals,merged,\
	icuin \
))
else
$(eval $(call gb_Library_use_externals,merged,\
	icui18n \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_libraries,merged,\
	AppleRemote \
))
$(eval $(call gb_Library_add_libs,merged,\
	-lobjc \
))
$(eval $(call gb_Library_use_system_darwin_frameworks,merged,\
	Carbon \
	Cocoa \
	CoreFoundation \
	CoreServices \
	QuickTime \
))
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_use_static_libraries,merged,\
	basebmp \
))

$(eval $(call gb_Library_use_libraries,merged,\
	libotouch \
))

$(eval $(call gb_Library_add_libs,merged,\
	-llog \
	-landroid \
	-llo-bootstrap \
))

$(eval $(call gb_Library_use_externals,merged,\
	fontconfig \
	freetype \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_Library_use_system_darwin_frameworks,merged,\
	CoreFoundation \
	UIKit \
))
endif

ifneq ($(ENABLE_LIBRSVG),NO)
$(eval $(call gb_Library_use_externals,merged,\
	cairo \
))
endif

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_Library_use_externals,merged,\
	gtk \
	telepathy \
))
endif

ifeq ($(ENABLE_DBUS),TRUE)
$(eval $(call gb_Library_use_externals,merged,\
	dbus \
))
endif

ifeq ($(ENABLE_LIBLANGTAG),YES)
$(eval $(call gb_Library_use_externals,merged,\
	glib \
	liblangtag \
))
endif

# vim: set noet sw=4 ts=4:
