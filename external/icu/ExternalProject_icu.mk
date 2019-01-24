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
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		export LIB="$(ILIB)" \
		&& CFLAGS="-FS -arch:SSE $(SOLARINC) $(gb_DEBUGINFO_FLAGS)" CPPFLAGS="$(SOLARINC)" CXXFLAGS="-FS -arch:SSE $(SOLARINC) $(gb_DEBUGINFO_FLAGS)" \
			INSTALL=`cygpath -m /usr/bin/install` \
			./runConfigureICU \
			$(if $(MSVC_USE_DEBUG_RUNTIME),--enable-debug --disable-release) \
			Cygwin/MSVC \
		&& $(MAKE) \
	,source)

else # $(OS)

icu_CFLAGS:=" \
	$(if $(filter iOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(SYSBASE),-I$(SYSBASE)/usr/include) \
	$(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter GCC,$(COM)),-fno-strict-aliasing) \
	$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS)) \
	$(if $(filter FUZZERS,$(BUILD_TYPE)),-DU_USE_STRTOD_L=0) \
	$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer)"
icu_CXXFLAGS:="$(CXXFLAGS) $(CXXFLAGS_CXX11) \
	$(if $(filter iOS,$(OS)),-DUCONFIG_NO_FILE_IO) \
	$(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter GCC,$(COM)),-fno-strict-aliasing) \
	$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS)) \
	$(if $(filter FUZZERS,$(BUILD_TYPE)),-DU_USE_STRTOD_L=0) \
	$(if $(filter ANDROID,$(OS)),-fvisibility=hidden -fno-omit-frame-pointer $(SOLARINC))"
icu_LDFLAGS:=" \
	$(if $(ENABLE_LTO),$(gb_LTOFLAGS)) \
	$(if $(filter TRUE,$(HAVE_LD_HASH_STYLE)),-Wl$(COMMA)--hash-style=$(WITH_LINKER_HASH_STYLE)) \
    $(if $(SYSBASE),-L../lib -L../../lib -L../stubdata -L../../stubdata -L$(SYSBASE)/usr/lib) \
    $(if $(filter TRUE,$(HAVE_LD_BSYMBOLIC_FUNCTIONS)), -Wl$(COMMA)-Bsymbolic-functions) \
    $(if $(filter ANDROID,$(OS)),$(gb_STDLIBS))"

# DATASUBDIR=data in cross-compiling case, because --disable-tools completely skips the
# data directory/doesn't build the requested library in that case (icu/source/Makefile.in)
# so we need to add it back to the list of subdirectories to build
$(call gb_ExternalProject_get_state_target,icu,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS=$(icu_CPPFLAGS) CFLAGS=$(icu_CFLAGS) \
		CXXFLAGS=$(icu_CXXFLAGS) LDFLAGS=$(icu_LDFLAGS) \
		./configure \
			--disable-layout --disable-samples \
			$(if $(filter FUZZERS,$(BUILD_TYPE)),--disable-release) \
			$(if $(CROSS_COMPILING),--disable-tools --disable-extras) \
			$(if $(filter iOS ANDROID,$(OS)),--disable-dyload) \
			$(if $(filter ANDROID,$(OS)),--disable-strict ac_cv_c_bigendian=no) \
			$(if $(filter SOLARIS AIX,$(OS)),--disable-64bit-libs) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),\
				--with-data-packaging=static --enable-static --disable-shared --disable-dyload,\
				--disable-static --enable-shared $(if $(filter ANDROID,$(OS)),--with-library-suffix=lo)) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)\
				--with-cross-build=$(WORKDIR_FOR_BUILD)/UnpackedTarball/icu/source) \
		&& $(MAKE) $(if $(CROSS_COMPILING),DATASUBDIR=data) \
		$(if $(filter MACOSX,$(OS)), \
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl \
				URELIB \
				$(EXTERNAL_WORKDIR)/source/lib/libicuuc$(gb_Library_DLLEXT).$(icu_VERSION) \
				$(EXTERNAL_WORKDIR)/source/lib/libicui18n$(gb_Library_DLLEXT).$(icu_VERSION) \
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl \
				OOO \
				$(EXTERNAL_WORKDIR)/source/lib/libicudata$(gb_Library_DLLEXT).$(icu_VERSION)) \
	,source)

endif

# vim: set noet sw=4 ts=4:
