# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libffi))

$(eval $(call gb_ExternalProject_register_targets,libffi,\
	build \
))

# set prefix so that it ends up in libffi.pc so that pkg-config in python3 works
# For a static Windows build, change CPPFLAGS to include -D_LIB and --disable-static
# Also remove the ExternalPackage in that case

libffi_WIN_PLATFORM := $(strip \
    $(if $(filter INTEL,$(CPUNAME)),32) \
    $(if $(filter X86_64,$(CPUNAME)),64) \
    $(if $(filter AARCH64,$(CPUNAME)),arm64) \
    )

$(call gb_ExternalProject_get_state_target,libffi,build):
	$(call gb_Trace_StartRange,libffi,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export LIB="$(ILIB)" && \
		MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--enable-option-checking=fatal \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter LINUX,$(OS)), \
			    --disable-shared \
			    CC="$(CC) -fvisibility=hidden" \
				--with-pic \
				--enable-portable-binary) \
			$(if $(filter WNT,$(OS)), \
			    --disable-static \
			    CC="$(gb_UnpackedTarball_workdir)/libffi/msvcc.sh -m$(libffi_WIN_PLATFORM)" \
			    CXX="$(gb_UnpackedTarball_workdir)/libffi/msvcc.sh -m$(libffi_WIN_PLATFORM)" \
				LD='link' \
				CPP='cl -nologo -EP' \
				CXXCPP='cl -nologo -EP' \
				CPPFLAGS="-DFFI_BUILDING_DLL $(SOLARINC)") \
			--prefix=$(gb_UnpackedTarball_workdir)/libffi/$(HOST_PLATFORM) \
			--disable-docs \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libffi,EXTERNAL)

# vim: set noet sw=4 ts=4:
