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

$(eval $(call gb_ExternalProject_use_autoconf,nss,build))

# The nss build system uses 'python', even recursively, so make it find our internal python if necessary.
nss_PYTHON := $(call gb_UnpackedTarball_get_dir,nss)/python
nss_SETUP_PYTHON := $(call gb_UnpackedTarball_get_dir,nss)/setup-python

$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalExecutable_get_dependencies,python) $(call gb_UnpackedTarball_get_target,gyp)
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	cp $(SRCDIR)/external/nss/python-cygwin-template $(nss_PYTHON)
	pythondir=$$($(call gb_ExternalExecutable_get_command,python) -c 'import sys; import os; sys.stdout.write(os.path.dirname(sys.executable))') \
		&& echo PATH=\"$$pythondir:\$$PATH\" >>$(nss_PYTHON)
	echo '$(call gb_ExternalExecutable_get_command,python)' \"$$\{args[@]\}\" >>$(nss_PYTHON)
	chmod +x $(nss_PYTHON)
	cp $(SRCDIR)/external/nss/setup-python $(nss_SETUP_PYTHON)
	chmod +x $(nss_SETUP_PYTHON)
	$(call gb_ExternalProject_run,build,\
			COMMA=$(COMMA) \
			PATH=$$(cygpath $(call gb_UnpackedTarball_get_dir,nss)):$$(cygpath $(call gb_UnpackedTarball_get_dir,gyp)):$$PATH \
			MAKE=$(MAKE) \
			NINJA='$(subst ','\'',$(NINJA))' \
			NSINSTALL='$(SRCDIR)/external/nss/nsinstall.py' \
			LIB="$(ILIB)" \
			RC="rc.exe $(SOLARINC)" \
			CL="-arch:SSE" \
			./build.sh -v --disable-tests --enable-libpkix \
				$(if $(filter X86_64,$(CPUNAME)),--target=x64,--target=ia32) \
				$(if $(ENABLE_DBGUTIL),,--opt) \
				$(if $(gb_Module_CURRENTMODULE_SYMBOLS_ENABLED),--symbols) \
		&& rm -f $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.a \
	,nss)
	for f in $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.dll.lib; do mv "$$f" "$${f%.dll.lib}".lib; done
	$(call gb_Trace_EndRange,nss,EXTERNAL)

# non-WNT gyp-based
# update nss_needs_ninja in configure.ac if the list changes
else ifneq (,$(filter FREEBSD LINUX NETBSD OPENBSD SOLARIS,$(OS)))

# The nss build system uses 'python', so make it find our internal python if necessary.
nss_PYTHON := $(call gb_UnpackedTarball_get_dir,nss)/python
nss_SETUP_PYTHON := $(call gb_UnpackedTarball_get_dir,nss)/setup-python

$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalExecutable_get_dependencies,python) $(call gb_UnpackedTarball_get_target,gyp)
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	echo "#! /bin/sh" > $(nss_PYTHON)
	pythondir=$$($(call gb_ExternalExecutable_get_command,python) -c 'import sys; import os; sys.stdout.write(os.path.dirname(sys.executable))') \
		&& echo PATH=\"$$pythondir:\$$PATH\" >>$(nss_PYTHON)
	echo '$(call gb_ExternalExecutable_get_command,python)' \"$$\@\" $(if $(ICECREAM_RUN), | sed 's/$(ICECREAM_RUN)//') >> $(nss_PYTHON)
	chmod +x $(nss_PYTHON)
	cp $(SRCDIR)/external/nss/setup-python $(nss_SETUP_PYTHON)
	chmod +x $(nss_SETUP_PYTHON)
	$(call gb_ExternalProject_run,build,\
			COMMA=$(COMMA) \
			PATH=$(call gb_UnpackedTarball_get_dir,nss):$(call gb_UnpackedTarball_get_dir,gyp):$$PATH \
			NINJA='$(subst ','\'',$(NINJA))' \
			./build.sh -v --disable-tests --enable-libpkix \
				$(if $(ENABLE_DBGUTIL),,--opt) \
				$(if $(COM_IS_CLANG),$(if $(filter -fsanitize=%,$(CC)),--no-zdefs)) \
		&& rm -f $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.a \
	,nss)
	$(call gb_Trace_EndRange,nss,EXTERNAL)

else # platforms which need(?) the old build system (feel free to port to the new system)
# make sure to specify NSPR_CONFIGURE_OPTS as env (before make command), so nss can append it's own defaults
# OTOH specify e.g. CC and NSINSTALL as arguments (after make command), so they will overrule nss makefile values
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Trace_StartRange,nss,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter ANDROID FREEBSD LINUX MACOSX,$(OS)),$(if $(filter X86_64,$(CPUNAME)),USE_64=1)) \
		$(if $(filter ANDROID,$(OS)),$(if $(filter AARCH64,$(CPUNAME)),USE_64=1)) \
		$(if $(filter iOS,$(OS)),$(if $(filter ARM64,$(CPUNAME)),USE_64=1)) \
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
			$(if $(filter ANDROID,$(OS)),OS_TARGET=Android OS_TARGET_RELEASE=16 ARCHFLAG="" DEFAULT_COMPILER=clang ANDROID_NDK=$(ANDROID_NDK_HOME) ANDROID_TOOLCHAIN_VERSION=$(ANDROID_GCC_TOOLCHAIN_VERSION) ANDROID_PREFIX=$(HOST_PLATFORM) ANDROID_SYSROOT=$(ANDROID_NDK_HOME)/sysroot ANDROID_TOOLCHAIN=$(ANDROID_BINUTILS_PREBUILT_ROOT)) \
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
	$(call gb_Trace_EndRange,nss,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
