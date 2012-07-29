# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,bridges))

$(eval $(call gb_Module_add_targets,bridges,\
	$(if $(filter TRUE,$(SOLAR_JAVA)),\
		Jar_java_uno \
		Library_java_uno \
	) \
	$(if $(and $(filter I,$(CPU)),$(filter C50 C52,$(COM))),\
		Library_cc50_solaris_intel \
	) \
	$(if $(filter C52S,$(COM)$(CPU)),\
		Library_cc50_solaris_sparc \
	) \
	$(if $(filter C52U,$(COM)$(CPU)),\
		Library_cc5_solaris_sparc64 \
	) \
	$(if $(filter GCCAIXP,$(COM)$(OS)$(CPU)),\
		Library_gcc3_aix_powerpc \
	) \
	$(if $(filter GCCIOSR,$(COM)$(OS)$(CPU)),\
		CustomTarget_gcc3_ios_arm \
		Library_gcc3_ios_arm \
	) \
	$(if $(filter GCCLINUXL,$(COM)$(OS)$(CPU)),\
		Library_gcc3_linux_alpha \
	) \
	$(if $(and $(filter GCCR,$(COM)$(CPU)),$(filter ANDROID LINUX,$(OS))),\
		Library_gcc3_linux_arm \
	) \
	$(if $(filter GCCLINUXH,$(COM)$(OS)$(CPU)),\
		Library_gcc3_linux_hppa \
	) \
	$(if $(filter GCCLINUXA,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_ia64 \
	) \
	$(if $(and $(filter GCCI,$(COM)$(CPU)),$(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS))),\
		Library_gcc3_linux_intel \
	) \
	$(if $(filter GCCLINUX6,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_m68k \
	) \
	$(if $(and $(filter GCCM,$(COM)$(CPU)),$(filter ANDROID LINUX,$(OS))),\
		Library_gcc3_linux_mips \
	) \
	$(if $(filter GCCLINUXPPOWERPC64,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_powerpc64 \
	) \
	$(if $(filter GCCLINUXPPOWERPC,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_powerpc \
	) \
	$(if $(filter GCCLINUX3S390,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_s390 \
	) \
	$(if $(filter GCCLINUX3S390X,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_linux_s390x \
	) \
	$(if $(filter GCCLINUXS,$(COM)$(OS)$(CPU)),\
		Library_gcc3_linux_sparc \
	) \
	$(if $(and $(filter GCCX,$(COM)$(CPU)),$(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS))),\
		Library_gcc3_linux_x86-64 \
	) \
	$(if $(filter GCCMACOSXI,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_macosx_intel \
	) \
	$(if $(filter GCCMACOSXP,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_macosx_powerpc \
	) \
	$(if $(filter GCCSOLARISI,$(COM)$(OS)$(CPU)$(CPUNAME)),\
		Library_gcc3_solaris_intel \
	) \
	$(if $(filter GCCSOLARISS,$(COM)$(OS)$(CPU)),\
		Library_gcc3_solaris_sparc \
	) \
	$(if $(filter WNTGCC,$(OS)$(COM)),\
		Library_mingw_intel \
	) \
	$(if $(filter MSCI,$(COM)$(CPU)),\
		Library_msvc_win32_intel \
	) \
	$(if $(filter MSCX,$(COM)$(CPU)),\
		Library_msvc_win32_x86-64 \
	) \
))

# vim: set noet sw=4 ts=4:
