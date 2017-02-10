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

ifeq ($(OS),WNT)

$(call gb_ExternalProject_get_state_target,pixman,build) :
	$(call gb_ExternalProject_run,build,\
		$(MAKE) -f Makefile.win32 MMX=on SSE2=on CFG=release \
	,pixman)

else

# ANDROID:
# The pixman-cpu.c code wants to read /proc/<pid>/auxv, but
# the Android headers don't define Elf32_auxv_t.
#
# Maybe we should instead just patch the arm_has_* booleans in
# pixman-cpu.c to be hardcoded as TRUE and patch out the run-time
# check?
$(call gb_ExternalProject_get_state_target,pixman,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,$(if $(filter ANDROID,$(OS)),--disable-shared,--disable-static)) \
		$(if $(filter ANDROID,$(OS)),--disable-arm-simd --disable-arm-neon --disable-arm-iwmmxt) \
		$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
		$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no)) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
