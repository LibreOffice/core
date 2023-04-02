# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,openssl))

$(eval $(call gb_ExternalProject_register_targets,openssl,\
	build \
))

# For multi-line conditionals, align the $(if and the corresponding ),
# putting the latter on a line of its own. Also put the "else" comma
# on a line of its own. Hopefully should make the logic more clear.

OPENSSL_PLATFORM := \
  $(if $(filter LINUX FREEBSD ANDROID,$(OS)),\
    $(if $(filter INTEL,$(CPUNAME)),\
      $(if $(filter GNU/kFreeBSD,$(shell uname)),debian-kfreebsd-i386,linux-elf)\
    ,\
      $(if $(filter X86_64,$(CPUNAME)),\
        $(if $(filter GNU/kFreeBSD,$(shell uname)),\
          debian-kfreebsd-amd64\
        ,\
          $(if $(filter TRUE, $(ENABLE_DBGUTIL)), debug-linux-generic64, linux-generic64) no-asm\
        )\
      ,\
        $(if $(filter TRUE, $(ENABLE_DBGUTIL)), debug-linux-generic32, linux-generic32)\
      )\
    )\
  ,\
    $(if $(filter SOLARIS,$(OS)),\
      $(if $(filter INTEL,$(CPUNAME)),solaris-x86-cc,\
        $(if $(filter X86_64,$(CPUNAME)),solaris64-x86_64-cc,solaris-sparcv9-cc)\
      )\
    ,\
      $(if $(filter iOS,$(OS)),\
        ios-armv7\
      ,\
        $(if $(filter WNT,$(OS)),\
          $(if $(filter INTEL,$(CPUNAME)),VC-WIN32)\
          $(if $(filter X86_64,$(CPUNAME)),VC-WIN64A)\
          $(if $(filter ARM64,$(CPUNAME)),VC-WIN64-ARM)\
        ,\
          $(if $(filter MACOSX,$(OS)),\
            $(if $(filter POWERPC,$(CPUNAME)),darwin-ppc-cc)\
            $(if $(filter INTEL,$(CPUNAME)),darwin-i386-cc)\
            $(if $(filter X86_64,$(CPUNAME)),darwin64-x86_64-cc)\
          )\
        )\
      )\
    )\
  )

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalProject_use_nmake,openssl,build))

$(call gb_ExternalProject_get_state_target,openssl,build):
	$(call gb_ExternalProject_run,build,\
		CONFIGURE_INSIST=1 $(PERL) Configure $(OPENSSL_PLATFORM) no-tests no-multilib \
		&& export PERL="$(shell cygpath -w $(PERL))" \
		&& nmake -f makefile \
	)

else
$(call gb_ExternalProject_get_state_target,openssl,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS && \
		$(if $(filter LINUX MACOSX FREEBSD ANDROID SOLARIS iOS,$(OS)), \
			./Configure, \
		$(if $(filter WNT,$(OS)), \
			$(PERL) Configure, \
			./config)) \
			$(OPENSSL_PLATFORM) no-dso no-shared no-tests no-multilib threads \
			$(if $(filter-out ANDROID iOS WNT,$(OS)), \
				$(if $(SYSBASE),-I$(SYSBASE)/usr/include -L$(SYSBASE)/usr/lib)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) build_libs \
			CC="$(CC) -fPIC \
				$(if $(filter TRUE, $(ENABLE_DBGUTIL)), -DPURIFY,) \
				$(if $(filter-out WNT MACOSX,$(OS)),-fvisibility=hidden)" \
	)
endif

# vim: set noet sw=4 ts=4:
