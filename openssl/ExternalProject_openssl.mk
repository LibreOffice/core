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
    $(if $(filter I,$(CPU)),\
      $(if $(filter GNU/kFreeBSD,$(shell uname)),debian-kfreebsd-i386,linux-elf)\
    ,\
      $(if $(filter X,$(CPU)),\
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
      $(if $(filter INTEL,$(CPUNAME)),\
        $(if $(filter X,$(CPU)),solaris64-x86_64-cc,solaris-x86-cc)\
      ,\
        solaris-sparcv9-cc\
      )\
    ,\
      $(if $(filter IOS,$(OS)),\
        ios-armv7\
      ,\
        $(if $(filter WNT,$(OS)),\
          $(if $(filter GCC,$(COM)),\
            mingw\
          ,\
            $(if $(filter I,$(CPU)),VC-WIN32,VC-WIN64A)\
          )\
        ,\
          $(if $(filter MACOSX,$(OS)),\
            $(if $(filter I,$(CPU)),darwin-i386-cc,darwin64-x86_64-cc)\
          )\
        )\
      )\
    )\
  )

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,openssl,build):
	$(call gb_ExternalProject_run,build,\
		export CC="$(shell cygpath -w $(CC))" \
		&& export PERL="$(shell cygpath -w $(PERL))" \
		&& export LIB="$(ILIB)" \
		&& $(PERL) Configure $(OPENSSL_PLATFORM) no-idea \
		&& cmd /c "ms\do_ms.bat $(PERL) $(OPENSSL_PLATFORM)" \
		&& unset MAKEFLAGS \
		&& nmake -f "ms\ntdll.mak" \
	)

else
$(call gb_ExternalProject_get_state_target,openssl,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS && \
		$(if $(filter LINUX MACOSX FREEBSD ANDROID SOLARIS IOS,$(OS)), \
			./Configure, \
		$(if $(filter WNT,$(OS)), \
			$(PERL) Configure, \
			./config)) \
			$(OPENSSL_PLATFORM) no-dso no-shared \
			$(if $(filter-out WNT,$(OS)),no-idea) \
			$(if $(filter-out ANDROID IOS WNT,$(OS)), \
				$(if $(SYSBASE),-I$(SYSBASE)/usr/include -L$(SYSBASE)/usr/lib)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) build_libs \
			CC="$(CC) -fPIC $(if $(filter-out WNT,$(OS)),\
			$(if $(filter TRUE,$(HAVE_GCC_VISIBILITY_FEATURE)),\
			-fvisibility=hidden))" \
	)
endif
# vim: set noet sw=4 ts=4:
