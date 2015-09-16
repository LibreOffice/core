# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,icu))

$(eval $(call gb_ExternalProject_register_targets,icu,\
	build \
))

icu_CPPFLAGS:="-DHAVE_GCC_ATOMICS=$(if $(filter TRUE,$(GCC_HAVE_BUILTIN_ATOMIC)),1,0)"

ifeq ($(OS),WNT)

# Note: runConfigureICU ignores everything following the platform name!
ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		export LIB="$(ILIB)" \
		&& CFLAGS="-FS -arch:SSE $(SOLARINC) $(gb_DEBUG_CFLAGS)" CPPFLAGS="$(SOLARINC)" CXXFLAGS="-FS -arch:SSE $(SOLARINC) $(gb_DEBUG_CFLAGS)" \
			INSTALL=`cygpath -m /usr/bin/install` \
			./runConfigureICU \
			$(if $(MSVC_USE_DEBUG_RUNTIME),--enable-debug --disable-release) \
			Cygwin/MSVC \
		&& $(MAKE) \
	,source)
else
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS=$(icu_CPPFLAGS) CFLAGS="-O -D_MT" CXXFLAGS="-O -D_MT" \
		LIBS="$(if $(MINGW_SHARED_GXXLIB),$(MINGW_SHARED_LIBSTDCPP))" \
		LDFLAGS="-L$(COMPATH)/lib -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 \
				$(if $(MINGW_SHARED_GCCLIB),-shared-libgcc)" \
		./configure \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			--with-cross-build=$(WORKDIR_FOR_BUILD)/UnpackedTarball/icu/source) \
			--disable-layout --disable-static --enable-shared --disable-samples \
		&& $(MAKE) \
		&&  for lib in icudata icuin icuuc; do \
			touch $$lib; \
			done \
	,source)
endif

else # $(OS)

icu_CFLAGS:=" \
	$(if $(filter IOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(SYSBASE),-I$(SYSBASE)/usr/include) \
	$(if $(debug),$(gb_DEBUG_CFLAGS),$(gb_COMPILEROPTFLAGS)) \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter GCC,$(COM)),-fno-strict-aliasing) \
	$(if $(filter $(true),$(gb_SYMBOL)),-g) \
	$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer)"
icu_CXXFLAGS:="$(CXXFLAGS) $(CXXFLAGS_CXX11) \
	$(if $(filter IOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(debug),$(gb_DEBUG_CFLAGS),$(gb_COMPILEROPTFLAGS)) \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter GCC,$(COM)),-fno-strict-aliasing) \
	$(if $(filter $(true),$(gb_SYMBOL)),-g) \
	$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer)"
icu_LDFLAGS:=" \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter TRUE,$(HAVE_LD_HASH_STYLE)),-Wl$(COMMA)--hash-style=$(WITH_LINKER_HASH_STYLE)) \
    $(if $(SYSBASE),-L../lib -L../../lib -L../stubdata -L../../stubdata -L$(SYSBASE)/usr/lib) \
    $(if $(filter TRUE,$(HAVE_LD_BSYMBOLIC_FUNCTIONS)),\
	    -Wl$(COMMA)-Bsymbolic-functions -Wl$(COMMA)--dynamic-list-cpp-new -Wl$(COMMA)--dynamic-list-cpp-typeinfo) \
    $(if $(filter ANDROID,$(OS)),-lgnustl_shared -lm)"

$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS=$(icu_CPPFLAGS) CFLAGS=$(icu_CFLAGS) \
		CXXFLAGS=$(icu_CXXFLAGS) LDFLAGS=$(icu_LDFLAGS) \
		./configure \
			--disable-layout --disable-samples \
			$(if $(CROSS_COMPILING),--disable-tools --disable-extras) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-dyload) \
			$(if $(filter ANDROID EMSCRIPTEN,$(OS)),--disable-strict ac_cv_c_bigendian=no) \
			$(if $(filter SOLARIS AIX,$(OS)),--disable-64bit-libs) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),\
				--enable-static --disable-shared,\
				--disable-static --enable-shared $(if $(filter ANDROID,$(OS)),--with-library-suffix=lo)) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)\
				--with-cross-build=$(WORKDIR_FOR_BUILD)/UnpackedTarball/icu/source) \
			$(if $(filter IOS,$(OS)), \
				--with-data-packaging=archive) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_icu)/source/lib/libicudata$(gb_Library_DLLEXT).$(icu_VERSION) \
			$(gb_Package_SOURCEDIR_icu)/source/lib/libicuuc$(gb_Library_DLLEXT).$(icu_VERSION) \
			$(gb_Package_SOURCEDIR_icu)/source/lib/libicui18n$(gb_Library_DLLEXT).$(icu_VERSION)) \
	,source)

endif

# vim: set noet sw=4 ts=4:
