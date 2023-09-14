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

# ANDROID:
# The pixman-cpu.c code wants to read /proc/<pid>/auxv, but
# the Android headers don't define Elf32_auxv_t.
#
# Maybe we should instead just patch the arm_has_* booleans in
# pixman-cpu.c to be hardcoded as TRUE and patch out the run-time
# check?
$(call gb_ExternalProject_get_state_target,pixman,build) :
	$(call gb_Trace_StartRange,pixman,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		$(if $(filter ANDROID,$(OS)),--disable-arm-simd --disable-arm-neon --disable-arm-a64-neon --disable-arm-iwmmxt) \
		$(gb_CONFIGURE_PLATFORMS) \
		$(if $(CROSS_COMPILING),$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no)) \
		$(if $(filter EMSCRIPTEN,$(OS)),CFLAGS="-O3 -pthread -msimd128") \
		$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,pixman,EXTERNAL)

# vim: set noet sw=4 ts=4:
