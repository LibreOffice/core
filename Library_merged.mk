# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,merged))

# gb_MERGEDLIBS is defined in solenv/gbuild/extensions/pre_MergedLibsList.mk
$(eval $(call gb_Library_use_library_objects,merged,\
	$(gb_MERGEDLIBS) \
))

$(eval $(call gb_Library_use_libraries,merged,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	$(if $(ENABLE_JAVA), \
		jvmaccess \
		jvmfwk) \
	sal \
	salhelper \
	ucbhelper \
	xmlreader \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_static_libraries,merged,\
	$(if $(filter LINUX,$(OS)),glxtest) \
	$(if $(filter unx,$(GUIBASE)),headless) \
))

$(eval $(call gb_Library_use_externals,merged,\
	icu_headers \
	icui18n \
	icuuc \
	boost_headers \
	boostdatetime \
	$(call gb_Helper_optional,DESKTOP,clucene) \
	$(call gb_Helper_optional,COLLADA2GLTF,collada2gltf) \
	cups \
	curl \
	dbus \
	expat \
	gconf \
	gio \
	glew \
	graphite \
	$(if $(ENABLE_GTK),gtk) \
	harfbuzz \
	hunspell \
	jpeg \
	lcms2 \
	$(call gb_Helper_optional,LIBATOMIC_OPS,libatomic_ops) \
	libeot \
	liblangtag \
	libxml2 \
	libxslt \
	mesa_headers \
	mythes \
	nss3 \
	$(call gb_Helper_optional,COLLADA2GLTF,opencollada_parser) \
	$(call gb_Helper_optional,COLLADA2GLTF,png) \
	telepathy \
	zlib \
))

ifeq (ALL,$(MERGELIBS))
$(eval $(call gb_Library_use_static_libraries,merged,\
	ulingu \
))
$(eval $(call gb_Library_use_externals,merged,\
	cppunit \
	hyphen \
	libexslt \
	libexttextcat \
	$(if $(filter-out ANDROID IOS,$(OS)),openldap) \
	$(call gb_Helper_optional,PYUNO,python) \
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
ifeq ($(ENABLE_OPENGL_CANVAS),TRUE)
$(eval $(call gb_Library_add_libs,merged,\
	-lm \
	-ldl \
	-lGL \
	-lGLU \
	-lpthread \
	-lrt \
	-lX11 \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,merged,\
	advapi32 \
	gdi32 \
	gdiplus \
	glu32 \
	imm32 \
	mpr \
	msimg32 \
	oldnames \
	ole32 \
	oleaut32 \
	opengl32 \
	shell32 \
	shlwapi \
	setupapi \
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
	OpenGL \
	QuickTime \
))
endif

# vim: set noet sw=4 ts=4:
