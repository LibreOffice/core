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

$(eval $(call gb_Library_use_libraries,merged,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
    i18nutil \
    jvmfwk \
	sal \
	salhelper \
	sax \
	sot \
	svl \
	tl \
	ucbhelper \
	utl \
	vcl \
	xmlreader \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_externals,merged,\
	icuuc \
	jpeg \
	libxml2 \
	telepathy \
	zlib \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_externals,merged,\
	berkeleydb \
))
endif

# gb_MERGEDLIBS is defined in solenv/gbuild/extensions/pre_MergedLibsList.mk
$(eval $(call gb_Library_use_library_objects,merged,\
	$(gb_MERGEDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,merged,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	uuid \
	winmm \
))
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_use_libraries,merged,\
	libotouch \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_libraries,merged,\
    objc \
    Cocoa \
))
endif

# vim: set noet sw=4 ts=4:
