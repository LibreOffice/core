# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,pixman))

$(eval $(call gb_ExternalProject_register_targets,pixman,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,pixman,\
	meson \
))

# We cannot use environment vars inside the meson cross-build file,
# so we're going to have to generate one on-the-fly.
# mungle variables into python list format
python_listify = '$(subst $(WHITESPACE),'$(COMMA)',$(strip $(1)))'
cross_c = $(call python_listify,$(gb_CC))
cross_cxx = $(call python_listify,$(gb_CXX))
cross_ld := $(call python_listify,$(subst -fuse-ld=,,$(USE_LD)))

define gb_pixman_cross_compile
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

$(call gb_ExternalProject_get_state_target,pixman,build) :
	$(call gb_Trace_StartRange,pixman,EXTERNAL)
	$(file >$(gb_UnpackedTarball_workdir)/pixman/cross-file.txt,$(gb_pixman_cross_compile))
	$(call gb_ExternalProject_run,build,\
		export PIXMAP_CFLAGS="\
			  $(if $(call gb_Module__symbols_enabled,pixman),$(gb_DEBUGINFO_FLAGS) ) \
			  $(if $(filter EMSCRIPTEN,$(OS)),-O3 -pthread -msimd128 ) \
			  $(if $(filter -fsanitize=undefined,$(CC)),-fno-sanitize=function ) \
			" && \
		$(MESON) setup --wrap-mode nofallback builddir \
			-Ddefault_library=$(if $(filter TRUE,$(DISABLE_DYNLOADING)),static,shared) \
			$(if $(call gb_Module__symbols_enabled,pixman),-Dc_args="$$PIXMAP_CFLAGS") \
			$(if $(filter ANDROID,$(OS)),-Darm-simd=disabled -Dneon=disabled -Da64-neon=disabled) \
			-Dbuildtype=$(if $(ENABLE_DBGUTIL),debug,$(if $(ENABLE_DEBUG),debugoptimized,release)) \
			-Dauto_features=disabled \
			-Dtests=disabled \
			$(if $(filter-out $(BUILD_PLATFORM),$(HOST_PLATFORM))$(WSL),--cross-file cross-file.txt) && \
		$(MESON) compile -C builddir \
			$(if $(verbose),--verbose) \
	)
	$(call gb_Trace_EndRange,pixman,EXTERNAL)

# vim: set noet sw=4 ts=4:
