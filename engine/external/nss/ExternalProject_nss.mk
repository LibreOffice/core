# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
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

$(eval $(call gb_ExternalProject_use_externals,nss,\
	zlib \
))

nss_PYTHON := $(if $(PYTHON_FOR_BUILD),$(PYTHON_FOR_BUILD),$(INSTROOT_FOR_BUILD)/$(LIBO_BIN_FOLDER)/python)

ifneq ($(filter LINUX WNT,$(OS)),)
$(call gb_ExternalProject_use_unpacked,nss,gyp)
endif

# Both gyp branches below ask for libpkix, which the gyp defaults leave out and
# coreconf built. Without it CERT_PKIXVerifyCert is a stub that always fails, so
# no certificate chain ever validates.

ifeq ($(OS),WNT)
# Windows builds nspr with coreconf's make, because nss's build.sh cannot build
# nspr in this environment. Both Windows routes below run coreconf with the same
# settings, and take the coreconf target to run as their argument.
nss_WNT_MAKE = \
	$(if $(MSVC_USE_DEBUG_RUNTIME),USE_DEBUG_RTL=1,BUILD_OPT=1) \
	$(if $(gb_Module_CURRENTMODULE_SYMBOLS_ENABLED), \
		MOZ_DEBUG_SYMBOLS=1 \
		MOZ_DEBUG_FLAGS=" " \
		OPT_CODE_SIZE=0) \
	OS_TARGET=WINNT \
	USE_SYSTEM_ZLIB=1 \
	$(if $(filter X86_64,$(CPUNAME)),USE_64=1) \
	$(if $(filter AARCH64,$(CPUNAME)),USE_64=1 CPU_ARCH=aarch64) \
	LIB="$(ILIB)" \
	XCFLAGS="$(SOLARINC) $(ZLIB_CFLAGS)" \
	NSPR_CONFIGURE_OPTS="$(gb_CONFIGURE_PLATFORMS)" \
	COMMA=$(COMMA) \
	$(if $(CROSS_COMPILING),\
		CROSS_COMPILE=1 \
		$(if $(filter AARCH64,$(CPUNAME)),CPU_ARCH=aarch64)) \
	$(MAKE) $(1) RC="rc.exe $(SOLARINC)" \
		NSINSTALL='$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py' \
		NSS_DISABLE_GTESTS=1 \
		NSS_DISABLE_CMD_TOOLS=1 \
		CCC="$(CXX)"

ifeq ($(CPUNAME),AARCH64)
# arm64 Windows builds all of nss with coreconf. nss's gyp build has no arm64
# Windows target: coreconf/config.gypi names the compiler platform and the
# 64-bit defines for ia32 and x64 alone, and gyp answers an arm64 request with
# its x86 default, which produces x86 libraries against an arm64 nspr.
$(call gb_ExternalProject_get_state_target,nss,build): \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(SRCDIR)/external/nss/nsinstall.py
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(call nss_WNT_MAKE,nss_build_all) \
	,nss)
	$(call gb_Trace_EndRange,nss,EXTERNAL)

else
# Every other Windows CPU builds nspr with coreconf, then compiles nss on top
# with gyp and ninja, run inline below. gyp comes from the gyp-next tarball
# unpacked beside nss.

ifeq ($(CPUNAME),X86_64)
python_arch_subdir=amd64
else ifeq ($(CPUNAME),AARCH64)
python_arch_subdir=arm64
endif

$(call gb_ExternalProject_get_state_target,nss,build): \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(SRCDIR)/external/nss/nsinstall.py
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(call nss_WNT_MAKE,build_nspr) \
		&& export PYEXE='$(gb_UnpackedTarball_workdir)/python3/PCbuild/$(python_arch_subdir)/python$(if $(MSVC_USE_DEBUG_RUNTIME),_d).exe' \
		&& root=$$(cygpath -u "$$("$$PYEXE" -c 'import os,sys;sys.stdout.write(os.path.realpath(sys.argv[1]))' "$$(cygpath -m ..)")") \
		&& cd "$$root/nss" \
		&& PATH="$(shell cygpath -u '$(SRCDIR)/external/gyp/bin'):$(shell cygpath -u '$(dir $(NINJA))'):$$PATH" \
			GYPDIR='$(gb_UnpackedTarball_workdir)/gyp' \
			VSPATH='$(VS_INSTALL_DIR)' \
			GYP_MSVS_OVERRIDE_PATH='$(VS_INSTALL_DIR)' \
			GYP_MSVS_VERSION='$(VS_YEAR)' \
			bash ./build.sh --msvc $(if $(MSVC_USE_DEBUG_RUNTIME),,-o) -t x64 -Ddisable_dbm=0 -Ddisable_libpkix=0 -Dsign_libs=0 -Ddisable_werror=1 -Ddisable_cmds=1 \
				--disable-tests \
				--with-nspr="$$(cygpath -m "$$root/dist/out")/include:$$(cygpath -m "$$root/dist/out")/lib" \
				--python="$$(cygpath -m "$$PYEXE")" \
		&& cp -f "$$root/dist/$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/lib"/*.dll ../dist/out/lib/ \
		&& for l in nss3 smime3 ssl3 nssutil3; do \
			cp -f "$$root/dist/$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/lib/$$l.dll.lib" "../dist/out/lib/$$l.lib"; \
		done \
	,nss)
	$(call gb_Trace_EndRange,nss,EXTERNAL)

endif

else ifeq ($(OS),LINUX)
# Linux builds nss with gyp and ninja through build.sh instead of coreconf.
# build.sh also builds nspr. It writes dist/Release, or dist/Debug for a debug
# build, with the nspr headers in a nspr subdirectory, so the recipe copies the
# tree to dist/out and lifts those headers up a level.
# CONFIG_SITE is cleared because distro autoconf site scripts (e.g. openSUSE's)
# set libdir to lib64, which makes nspr's "make install" put its libraries in
# dist/*/lib64 instead of dist/*/lib where the nss libraries and packaging
# expect them.
$(call gb_ExternalProject_get_state_target,nss,build): \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(SRCDIR)/external/nss/nsinstall.py
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	+$(call gb_ExternalProject_run,build,\
		PATH="$(SRCDIR)/external/gyp/bin:$$PATH" \
		CONFIG_SITE= \
		GYPDIR="$(gb_UnpackedTarball_workdir)/gyp" \
		PYEXE="$(nss_PYTHON)" \
		COMMA=$(COMMA) \
		CC="$(CC) $(gb_DEBUGINFO_FLAGS) $(if $(filter -fsanitize=undefined,$(CC)),-fno-sanitize=function)" \
		CXX="$(CXX) $(gb_DEBUGINFO_FLAGS)" \
		bash ./build.sh $(if $(LOADLIMIT),-l $(LOADLIMIT)) --disable-tests --enable-legacy-db --enable-libpkix -Dsign_libs=0 -Ddisable_werror=1 -Ddisable_cmds=1 $(if $(ENABLE_DBGUTIL),,--opt) \
			--python="$(nss_PYTHON)" \
			$(if $(and $(filter TRUE,$(COM_IS_CLANG)),$(filter -fsanitize=%,$(CC)),$(if $(filter -shared-libsan,$(CC) $(LDFLAGS)),,x)),--no-zdefs) \
		&& rm -rf ../dist/out \
		&& cp -a ../dist/$(if $(ENABLE_DBGUTIL),Debug,Release) ../dist/out \
		&& if [ -d ../dist/out/include/nspr ]; then \
			cp -a ../dist/out/include/nspr/. ../dist/out/include/; fi \
		&& rm -f ../dist/out/lib/*.a \
	,nss)
	$(call gb_Trace_EndRange,nss,EXTERNAL)

else # OS!=WNT and OS!=LINUX
# make sure to specify NSPR_CONFIGURE_OPTS as env (before make command), so nss can append it's own defaults
# OTOH specify e.g. CC and NSINSTALL as arguments (after make command), so they will overrule nss makefile values
$(call gb_ExternalProject_get_state_target,nss,build): \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(SRCDIR)/external/nss/nsinstall.py
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter ANDROID FREEBSD LINUX MACOSX iOS,$(OS)),$(if $(filter X86_64,$(CPUNAME)),USE_64=1 CPU_ARCH=x86_64)) \
		$(if $(filter AARCH64,$(CPUNAME)),USE_64=1 CPU_ARCH=aarch64) \
		$(if $(filter POWERPC64,$(CPUNAME)),USE_64=1 CPU_ARCH=ppc64le) \
		$(if $(filter MACOSX,$(OS)),\
			MACOS_SDK_DIR=$(MACOSX_SDK_PATH) \
			NSS_USE_SYSTEM_SQLITE=1) \
		$(if $(filter LINUX,$(OS)),$(if $(ENABLE_DBGUTIL),,BUILD_OPT=1)) \
		$(if $(filter SOLARIS,$(OS)),NS_USE_GCC=1) \
		$(if $(filter ARM,$(CPUNAME)),NSS_DISABLE_ARM32_NEON=1) \
		NSPR_CONFIGURE_OPTS="$(gb_CONFIGURE_PLATFORMS)" \
		$(if $(CROSS_COMPILING),CROSS_COMPILE=1) \
		NSDISTMODE=copy \
		$(MAKE) \
			AR="$(AR)" \
			RANLIB="$(RANLIB)" \
			NMEDIT="$(NM)edit" \
			COMMA=$(COMMA) \
			CC="$(CC) $(gb_DEBUGINFO_FLAGS) \
				$(if $(filter -fsanitize=undefined,$(CC)),-fno-sanitize=function) \
				$(if $(filter iOS,$(OS)), -DNSS_STATIC_SOFTOKEN=1 -DNSS_STATIC_FREEBL=1 -DNSS_STATIC_PKCS11=1) \
				$(if $(filter ANDROID,$(OS)), -DSQLITE_DISABLE_LFS=1)" \
			CCC="$(CXX) $(gb_DEBUGINFO_FLAGS)" \
			$(if $(CROSS_COMPILING),NSINSTALL="$(if $(filter MACOSX,$(OS_FOR_BUILD)),xcrun python3,$(call gb_ExternalExecutable_get_command,python)) $(SRCDIR)/external/nss/nsinstall.py") \
			$(if $(filter ANDROID,$(OS)),OS_TARGET=Android OS_TARGET_RELEASE=$(ANDROID_API_LEVEL) ARCHFLAG="" DEFAULT_COMPILER=clang ANDROID_NDK=$(ANDROID_NDK_DIR) ANDROID_TOOLCHAIN_VERSION=obsolete ANDROID_PREFIX=$(HOST_PLATFORM) ANDROID_SYSROOT=$(ANDROID_NDK_DIR)/sysroot) \
			NSS_DISABLE_GTESTS=1 \
			NSS_DISABLE_CMD_TOOLS=1 \
			nss_build_all \
		&& rm -f $(gb_UnpackedTarball_workdir)/nss/dist/out/lib/*.a \
		$(if $(filter MACOSX,$(OS)),\
			&& chmod u+w $(gb_UnpackedTarball_workdir)/nss/dist/out/lib/*.dylib \
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
	$(call gb_Trace_EndRange,nss,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
