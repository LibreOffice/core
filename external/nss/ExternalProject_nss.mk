# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,nss))

# nss build calls configure for nspr itself - if for some reason the configure step should be split out,
# make sure to create config.status (aka run configure) in dir specified with OBJDIR_NAME (nspr/out)
$(eval $(call gb_ExternalProject_register_targets,nss,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		$(if $(MSVC_USE_DEBUG_RUNTIME),USE_DEBUG_RTL=1,BUILD_OPT=1) \
		MOZ_MSVCVERSION=9 OS_TARGET=WIN95 \
		$(if $(filter X86_64,$(CPUNAME)),USE_64=1) \
		LIB="$(ILIB)" \
		XCFLAGS="-arch:SSE $(SOLARINC)" \
		$(MAKE) -j1 nss_build_all RC="rc.exe $(SOLARINC)" \
			NSINSTALL='$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py' \
			NSS_DISABLE_GTESTS=1 \
	,nss)

else # OS!=WNT
# make sure to specify NSPR_CONFIGURE_OPTS as env (before make command), so nss can append it's own defaults
# OTOH specify e.g. CC and NSINSTALL as arguments (after make command), so they will overrule nss makefile values
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter FREEBSD LINUX MACOSX,$(OS)),$(if $(filter X86_64,$(CPUNAME)),USE_64=1)) \
		$(if $(filter iOS,$(OS)),\
			$(if $(filter arm64,$(CC)),USE_64=1)) \
		$(if $(filter MACOSX,$(OS)),\
			$(if $(filter-out POWERPC,$(CPUNAME)),MACOS_SDK_DIR=$(MACOSX_SDK_PATH)) \
			NSS_USE_SYSTEM_SQLITE=1) \
		$(if $(filter LINUX,$(OS)),$(if $(ENABLE_DBGUTIL),,BUILD_OPT=1)) \
		$(if $(filter SOLARIS,$(OS)),NS_USE_GCC=1) \
		$(if $(CROSS_COMPILING),\
			$(if $(filter MACOSXPOWERPC,$(OS)$(CPUNAME)),CPU_ARCH=ppc) \
			$(if $(filter iOS-ARM,$(OS)-$(CPUNAME)),CPU_ARCH=arm) \
			NSPR_CONFIGURE_OPTS="--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)") \
		NSDISTMODE=copy \
		$(MAKE) -j1 AR="$(AR)" \
			RANLIB="$(RANLIB)" \
			NMEDIT="$(NM)edit" \
			COMMA=$(COMMA) \
			CC="$(CC)$(if $(filter ANDROID,$(OS)), -D_PR_NO_LARGE_FILES=1 -DSQLITE_DISABLE_LFS=1)" CCC="$(CXX)" \
			$(if $(CROSS_COMPILING),NSINSTALL="$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py") \
			$(if $(filter ANDROID,$(OS)),OS_TARGET=Android OS_TARGET_RELEASE=14 ARCHFLAG="" DEFAULT_COMPILER=clang ANDROID_NDK=$(ANDROID_NDK_HOME) ANDROID_TOOLCHAIN_VERSION=$(ANDROID_GCC_TOOLCHAIN_VERSION) ANDROID_PREFIX=$(HOST_PLATFORM) ANDROID_SYSROOT=$(SYSBASE) ANDROID_TOOLCHAIN=$(ANDROID_BINUTILS_PREBUILT_ROOT)) \
			NSS_DISABLE_GTESTS=1 \
			nss_build_all \
		&& rm -f $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.a \
		$(if $(filter MACOSX,$(OS)),\
			&& chmod u+w $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.dylib \
			&& $(PERL) \
				$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libfreebl3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libnspr4.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libnss3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libnssckbi.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libnssdbm3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libnssutil3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libplc4.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libplds4.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libsmime3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libsoftokn3.dylib \
				$(EXTERNAL_WORKDIR)/dist/out/lib/libssl3.dylib) \
	,nss)

endif

# vim: set noet sw=4 ts=4:
