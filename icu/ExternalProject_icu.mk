# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,icu))

$(eval $(call gb_ExternalProject_use_unpacked,icu,icu))

$(eval $(call gb_ExternalProject_use_package,icu,icu_data))

$(eval $(call gb_ExternalProject_register_targets,icu,\
	build \
))

icu_CPPFLAGS:="-DHAVE_GCC_ATOMICS=$(if $(filter TRUE,$(GCC_HAVE_BUILTIN_ATOMIC)),1,0)"

ifeq ($(OS),WNT)

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		export LIB="$(ILIB)" \
		&& CFLAGS="$(SOLARINC)" CPPFLAGS="$(SOLARINC)" CXXFLAGS="$(SOLARINC)" ./runConfigureICU Cygwin/MSVC \
		&& $(MAKE) \
	,source)
else
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS=$(icu_CPPFLAGS) CFLAGS="-O -D_MT" CXXFLAGS="-O -D_MT" \
		LIBS="$(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED_LIBSTDCPP))" \
		LDFLAGS="-L$(COMPATH)/lib -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 \
				$(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
		./configure
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			--with-cross-build=$(subst $(INPATH),$(INPATH_FOR_BUILD),$(call gb_UnpackedTarball_get_dir,icu))/source) \
			--enable-layout --disable-static --enable-shared --disable-samples \
		&& $(MAKE) \
		&&  for lib in icudata icuin icuuc icule icutu; do \
			@touch $$lib; \
			done \
	,source)
endif

else # $(OS)

icu_CFLAGS:="$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer) \
	$(if $(filter IOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(SYSBASE),-I$(SYSBASE)/usr/include) \
	$(if $(debug),-g,\
	$(if $(filter GCC,$(COM)),-O2 -fno-strict-aliasing,-O))"
icu_CXXFLAGS:="$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer) \
	$(if $(filter IOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(debug),-g,\
	$(if $(filter GCC,$(COM)),-O2 -fno-strict-aliasing,-O))"
icu_LDFLAGS:="$(if $(filter TRUE,$(HAVE_LD_HASH_STYLE)),-Wl$(COMMA)--hash-style=$(WITH_LINKER_HASH_STYLE)) \
	    $(if $(SYSBASE),-L../lib -L../../lib -L../stubdata -L../../stubdata -L$(SYSBASE)/usr/lib) \
	    $(if $(filter TRUE,$(HAVE_LD_BSYMBOLIC_FUNCTIONS)),\
	    -Wl$(COMMA)-Bsymbolic-functions -Wl$(COMMA)--dynamic-list-cpp-new -Wl$(COMMA)--dynamic-list-cpp-typeinfo) \
	    $(if $(filter ANDROID,$(OS)),-lgnustl_shared -lm)"

$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS=$(icu_CPPFLAGS) CFLAGS=$(icu_CFLAGS) \
		CXXFLAGS=$(icu_CXXFLAGS) LDFLAGS=$(icu_LDFLAGS) \
		./configure \
			--enable-layout --disable-samples \
			$(if $(filter IOS ANDROID,$(OS)),--disable-dyload) \
			$(if $(filter ANDROID,$(OS)),--disable-strict) \
			$(if $(filter SOLARIS AIX,$(OS)),--disable-64bit-libs) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),\
				--enable-static --disable-shared,\
				--disable-static --enable-shared $(if $(filter ANDROID,$(OS)),--with-library-suffix=lo)) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)\
				--with-cross-build=$(subst $(INPATH),$(INPATH_FOR_BUILD),$(call gb_UnpackedTarball_get_dir,icu))/source)\
		&& $(MAKE) \
	,source)

endif

# vim: set noet sw=4 ts=4:
