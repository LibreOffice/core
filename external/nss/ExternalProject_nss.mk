# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,nss))

$(eval $(call gb_ExternalProject_register_targets,nss,\
	configure \
	build \
))

$(call gb_ExternalProject_get_state_target,nss,configure):
	$(call gb_ExternalProject_run,configure,\
		$(if $(filter MSC,$(COM)),LIB="$(ILIB)") \
		nspr/configure --includedir=$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/out/include \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MSC-X86_64,$(COM)-$(CPUNAME)),--enable-64bit) \
	,,nss_configure.log)

ifeq ($(OS),WNT)
ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalProject_get_state_target,nss,configure) $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		$(if $(MSVC_USE_DEBUG_RUNTIME),USE_DEBUG_RTL=1,BUILD_OPT=1) \
		MOZ_MSVCVERSION=9 OS_TARGET=WIN95 \
		$(if $(filter X86_64,$(CPUNAME)),USE_64=1) \
		LIB="$(ILIB)" \
		XCFLAGS="$(SOLARINC)" \
		$(MAKE) -j1 nss_build_all RC="rc.exe $(SOLARINC)" \
			NSINSTALL='$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py' \
	,nss)


else
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalProject_get_state_target,nss,configure) $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		$(MAKE) -j1 nss_build_all \
			NS_USE_GCC=1 \
			CC="$(CC) $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			CXX="$(CXX) $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			OS_LIBS="-ladvapi32 -lws2_32 -lmswsock -lwinmm $(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED_LIBSTDCPP))" \
			LDFLAGS="" \
			PATH="$(PATH)" \
			RANLIB="$(RANLIB)" \
			OS_TARGET=WINNT RC="$(WINDRES)" OS_RELEASE="5.0" \
			IMPORT_LIB_SUFFIX=dll.a \
			NSPR_CONFIGURE_OPTS="--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --enable-shared --disable-static" \
			NSINSTALL="$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py" \
		&& rm -f $(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/out/lib/*.a \
	,nss)

endif
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,nss,build): $(call gb_ExternalProject_get_state_target,nss,configure) $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter FREEBSD LINUX MACOSX,$(OS)),$(if $(filter X86_64,$(CPUNAME)),USE_64=1)) \
		$(if $(filter MACOSX,$(OS)),MACOS_SDK_DIR=$(MACOSX_SDK_PATH) \
			$(if $(filter 1050,$(MAC_OS_X_VERSION_MIN_REQUIRED)),,NSS_USE_SYSTEM_SQLITE=1)) \
		$(if $(filter SOLARIS,$(OS)),NS_USE_GCC=1) \
		$(if $(filter YES,$(CROSS_COMPILING)),\
		$(if $(filter MACOSXPOWERPC,$(OS)$(CPUNAME)),CPU_ARCH=ppc) \
		NSINSTALL="$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/nss/nsinstall.py") \
		NSDISTMODE=copy \
		$(MAKE) -j1 AR="$(AR)" RANLIB="$(RANLIB)" NMEDIT="$(NM)edit" nss_build_all \
		&& rm -f $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.a \
		$(if $(filter MACOSX,$(OS)),\
			&& chmod u+w $(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib/*.dylib \
			&& $(PERL) \
				$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libfreebl3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libnspr4.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libnss3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libnssckbi.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libnssdbm3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libnssutil3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libplc4.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libplds4.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libsmime3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libsoftokn3.dylib \
				$(gb_Package_SOURCEDIR_nss)/dist/out/lib/libssl3.dylib) \
	,nss)

endif

# vim: set noet sw=4 ts=4:
