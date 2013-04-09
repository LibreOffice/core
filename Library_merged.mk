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
	i18nlangtag \
	jvmaccess \
	jvmfwk \
	sal \
	salhelper \
	tl \
	ucbhelper \
	xmlreader \
	$(gb_UWINAPI) \
))

ifneq ($(OS),IOS)
$(eval $(call gb_Library_use_static_libraries,merged,\
	writerperfect \
))
endif

$(eval $(call gb_Library_use_externals,merged,\
	boostdatetime \
	$(if $(filter-out IOS,$(OS)),cdr) \
	$(call gb_Helper_optional,DESKTOP,clucene) \
	cmis \
	cppunit \
	cups \
	curl \
	expat \
	hunspell \
	hyphen \
	icui18n \
	icule \
	icuuc \
	jpeg \
	lcms2 \
	libxml2 \
	$(call gb_Helper_optional,DESKTOP,libxslt) \
	$(if $(filter-out IOS,$(OS)),lpsolve) \
	$(if $(filter-out IOS,$(OS)),mspub) \
	$(if $(filter-out IOS,$(OS)),mwaw) \
	mythes \
	nss3 \
	openssl \
	orcus \
	$(call gb_Helper_optional,PYUNO,python) \
	$(if $(filter-out IOS,$(OS)),wpd) \
	$(if $(filter-out IOS,$(OS)),wpg) \
	$(if $(filter-out IOS,$(OS)),wps) \
	$(if $(filter-out IOS,$(OS)),visio) \
	zlib \
))

$(eval $(call gb_Library_use_static_libraries,merged,\
	sax_shared \
	ulingu \
))

ifeq ($(ENABLE_GRAPHITE),TRUE)
$(eval $(call gb_Library_use_externals,merged,\
	graphite \
))
endif

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_use_externals,merged,\
	fontconfig \
	freetype \
))
$(eval $(call gb_Library_add_libs,merged,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
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
# prevent warning spamming
$(eval $(call gb_Library_add_ldflags,merged,\
	/ignore:4049 \
	/ignore:4217 \
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
	liblangtag \
))
endif

# vim: set noet sw=4 ts=4:
