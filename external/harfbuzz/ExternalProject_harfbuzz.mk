# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,harfbuzz))

$(eval $(call gb_ExternalProject_register_targets,harfbuzz,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,harfbuzz,\
	icu \
	graphite \
	meson \
))

# We cannot use environment vars inside the meson cross-build file,
# so we're going to have to generate one on-the-fly.
# mungle variables into python list format
python_listify = '$(subst $(WHITESPACE),'$(COMMA)',$(strip $(1)))'
cross_c = $(call python_listify,$(gb_CC))
cross_cxx = $(call python_listify,$(gb_CXX))
cross_ld := $(call python_listify,$(subst -fuse-ld=,,$(USE_LD)))

define gb_harfbuzz_cross_compile
[binaries]
c = [$(cross_c)]
cpp = [$(cross_cxx)]
c_ld = [$(cross_ld)]
cpp_ld = [$(cross_ld)]
ar = '$(AR)'
strip = '$(STRIP)'
# TODO: this is pretty ugly...
[host_machine]
system = '$(if $(filter WNT,$(OS)),windows,$(if $(filter MACOSX,$(OS)),darwin,$(if $(filter ANDROID,$(OS)),android,linux)))'
cpu_family = '$(subst X86_64,x86_64,$(RTL_ARCH))'
cpu = '$(if $(filter x86,$(RTL_ARCH)),i686,$(if $(filter X86_64,$(RTL_ARCH)),x86_64,$(if $(filter AARCH64,$(RTL_ARCH)),aarch64,armv7)))'
endian = '$(ENDIANNESS)'
endef

# cannot use CROSS_COMPILING as condition since we have cross-compilation "light" for cases where
# the builder can run the host binaries, like for example when compiling for win 32bit on win 64bit
$(call gb_ExternalProject_get_state_target,harfbuzz,build) : | $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Trace_StartRange,harfbuzz,EXTERNAL)
	$(file >$(gb_UnpackedTarball_workdir)/harfbuzz/cross-file.txt,$(gb_harfbuzz_cross_compile))
	$(call gb_ExternalProject_run,build,\
		PKG_CONFIG_PATH="${PKG_CONFIG_PATH}$(LIBO_PATH_SEPARATOR)$(gb_UnpackedTarball_workdir)/graphite$(if $(SYSTEM_ICU),,$(LIBO_PATH_SEPARATOR)$(gb_UnpackedTarball_workdir)/icu)" \
		PYTHONWARNINGS= \
		$(MESON) setup builddir \
			-Ddefault_library=static -Dbuildtype=$(if $(ENABLE_DBGUTIL),debug,$(if $(ENABLE_DEBUG),debugoptimized,release \
			$(if $(call gb_Module__symbols_enabled,harfbuzz),$(addsuffix "$(strip $(gb_DEBUGINFO_FLAGS))",-Dc_args= -Dcpp_args=)))) \
			-Dauto_features=disabled \
			-Dcpp_std=$(subst -std:,,$(subst -std=,,$(filter -std%,$(CXXFLAGS_CXX11)))) \
			-Dtests=disabled \
			-Dutilities=disabled \
			-Dicu=enabled \
			-Dicu_builtin=true \
			-Dgraphite2=enabled \
			$(if $(filter-out $(BUILD_PLATFORM),$(HOST_PLATFORM))$(WSL),--cross-file cross-file.txt) && \
		$(MESON) compile -C builddir lib \
			$(if $(verbose),--verbose) \
	)
	$(call gb_Trace_EndRange,harfbuzz,EXTERNAL)

# vim: set noet sw=4 ts=4:
