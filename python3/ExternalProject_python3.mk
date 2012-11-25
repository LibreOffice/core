# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,python3))

$(eval $(call gb_ExternalProject_use_unpacked,python3,python3))

$(eval $(call gb_ExternalProject_use_external,python3,expat))

$(eval $(call gb_ExternalProject_register_targets,python3,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

# TODO: using Debug configuration and related mangling of pyconfig.h

# at least for MSVC 2008 it is necessary to clear MAKEFLAGS because
# nmake is invoked
$(call gb_ExternalProject_get_state_target,python3,build) :
ifeq ($(VCVER),110)
	cd $(EXTERNAL_WORKDIR)/PCbuild \
	&& MAKEFLAGS= MSBuild.exe pcbuild.sln /t:Build /p:Configuration=Release /p:PlatformToolset=v110 \
	&& cd $(EXTERNAL_WORKDIR) \
	&& ln -s $(if $(filter 90,$(VCVER)),PC/VS9.0,PCbuild) LO_lib \
	&& touch $@
else ifeq ($(VCVER),100)
	cd $(EXTERNAL_WORKDIR)/PCbuild \
	&& MAKEFLAGS= MSBuild.exe pcbuild.sln /t:Build /p:Configuration=Release /ToolsVersion:4.0 \
	&& cd $(EXTERNAL_WORKDIR) \
	&& ln -s $(if $(filter 90,$(VCVER)),PC/VS9.0,PCbuild) LO_lib \
	&& touch $@
else ifeq ($(VCVER),90)
	cd $(EXTERNAL_WORKDIR)/PC/VS9.0 \
	&& MAKEFLAGS= $(COMPATH)/vcpackages/vcbuild.exe pcbuild.sln "Release|$(if $(filter INTEL,$(CPUNAME)),Win32,x64)" \
	&& cd $(EXTERNAL_WORKDIR) \
	&& ln -s $(if $(filter 90,$(VCVER)),PC/VS9.0,PCbuild) LO_lib \
	&& touch $@
endif

else

# this was added in 2004, hopefully is obsolete now (and why only intel anyway)? $(if $(filter SOLARIS-INTEL,$(OS)$(CPUNAME)),--disable-ipv6)

# --with-system-expat: this should find the one in the solver (or system)

# create a symlink "LO_lib" because the .so are in a directory with platform
# specific name like build/lib.linux-x86_64-3.3

$(call gb_ExternalProject_get_state_target,python3,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		--with-system-expat \
		$(if $(strip $(VALGRIND_CFLAGS)),--with-valgrind) \
		--prefix=/python-inst \
		$(if $(filter AIX,$(OS)),--disable-ipv6 --with-threads CFLAGS="-g0") \
		$(if $(filter WNT-GCC,$(OS)-$(COM)),--with-threads ac_cv_printf_zd_format=no) \
		$(if $(filter MACOSX,$(OS)), \
			--enable-universalsdk=$(MACOSX_SDK_PATH) --with-universal-archs=32-bit --enable-framework=/@__________________________________________________OOO --with-framework-name=LibreOfficePython, \
			--enable-shared \
		) \
		CC="$(strip $(CC) \
			$(if $(filter YES,$(SYSTEM_OPENSSL)),, -I$(OUTDIR)/inc/external) \
			$(if $(filter YES,$(SYSTEM_EXPAT)),, -I$(OUTDIR)/inc/external/expat) \
			$(if $(SYSBASE), -I$(SYSBASE)/usr/include) \
			)" \
		LDFLAGS="$(strip $(LDFLAGS) \
			$(if $(filter YES,$(SYSTEM_OPENSSL)),, -L$(OUTDIR)/lib) \
			$(if $(filter YES,$(SYSTEM_EXPAT)),, -L$(OUTDIR)/lib) \
			$(if $(SYSBASE), -L$(SYSBASE)/usr/lib) \
			$(if $(filter WNT-GCC,$(OS)-$(COM)), -shared-libgcc \
				$(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols)) \
			)" \
	&& MAKEFLAGS=$(if $(VERBOSE)$(verbose),,s) $(MAKE) $(if $(filter MACOSX,$(OS)), DESTDIR=$(EXTERNAL_WORKDIR)/python-inst install) \
	&& ln -s build/lib.* LO_lib \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
