# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,python3))

$(eval $(call gb_ExternalProject_use_externals,python3,\
	expat \
	openssl \
	zlib \
))

$(eval $(call gb_ExternalProject_register_targets,python3,\
	build \
	$(if $(filter MACOSX,$(OS)),\
		fixscripts \
		fixinstallnames \
		executables \
	) \
))

ifeq ($(OS)$(COM),WNTMSC)

# TODO: using Debug configuration and related mangling of pyconfig.h

# at least for MSVC 2008 it is necessary to clear MAKEFLAGS because
# nmake is invoked
$(call gb_ExternalProject_get_state_target,python3,build) :
	$(call gb_ExternalProject_run,build,\
		MAKEFLAGS= MSBuild.exe pcbuild.sln /t:Build \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			/p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			$(if $(filter 100,$(VCVER)), \
				/ToolsVersion:4.0, \
				/p:PlatformToolset=v110 /p:VisualStudioVersion=11.0) \
		&& cd $(EXTERNAL_WORKDIR) \
		&& ln -s PCbuild LO_lib \
	,PCBuild)

else

# this was added in 2004, hopefully is obsolete now (and why only intel anyway)? $(if $(filter SOLARIS-INTEL,$(OS)$(CPUNAME)),--disable-ipv6)

# --with-system-expat: this should find the one in the solver (or system)

# create a symlink "LO_lib" because the .so are in a directory with platform
# specific name like build/lib.linux-x86_64-3.3

python3_cflags = $(ZLIB_CFLAGS)
ifeq ($(ENABLE_VALGRIND),TRUE)
    python3_cflags += $(VALGRIND_CFLAGS)
endif

$(call gb_ExternalProject_get_state_target,python3,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(filter TRUE,$(ENABLE_VALGRIND)),--with-valgrind) \
		--prefix=/python-inst \
		$(if $(filter MACOSX,$(OS)),,--with-system-expat) \
		$(if $(filter AIX,$(OS)), \
			--disable-ipv6 --with-threads OPT="-g0 -fwrapv -O3 -Wall", \
			$(if $(gb_Module_CURRENTMODULE_DEBUG_ENABLED), \
				OPT="$(gb_COMPILERNOOPTFLAGS) $(gb_DEBUG_CFLAGS)")) \
		$(if $(filter WNT-GCC,$(OS)-$(COM)),--with-threads ac_cv_printf_zd_format=no) \
		$(if $(filter MACOSX,$(OS)), \
			$(if $(filter INTEL POWERPC,$(CPUNAME)),--enable-universalsdk=$(MACOSX_SDK_PATH) --with-universal-archs=32-bit) --enable-framework=/@__________________________________________________OOO --with-framework-name=LibreOfficePython, \
			--enable-shared \
		) \
		CC="$(strip $(CC) \
			$(if $(filter NO,$(SYSTEM_OPENSSL)),-I$(call gb_UnpackedTarball_get_dir,openssl)/include) \
			$(if $(and $(filter NO,$(SYSTEM_OPENSSL)), $(filter-out YES,$(DISABLE_OPENSSL))),-I$(call gb_UnpackedTarball_get_dir,openssl)/include) \
			$(if $(filter NO,$(SYSTEM_EXPAT)),-I$(call gb_UnpackedTarball_get_dir,expat)/lib) \
			$(if $(SYSBASE), -I$(SYSBASE)/usr/include) \
			)" \
		$(if $(python3_cflags),CFLAGS='$(python3_cflags)') \
		LDFLAGS="$(strip $(LDFLAGS) \
			$(if $(filter YES,$(SYSTEM_OPENSSL)),, -L$(OUTDIR)/lib) \
			$(if $(filter YES,$(SYSTEM_EXPAT)),, -L$(OUTDIR)/lib) \
			$(if $(SYSBASE), -L$(SYSBASE)/usr/lib) \
			$(if $(filter WNT-GCC,$(OS)-$(COM)), -shared-libgcc \
				$(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols)) \
			)" \
		&& MAKEFLAGS= $(MAKE) \
			$(if $(filter MACOSX,$(OS)),DESTDIR=$(EXTERNAL_WORKDIR)/python-inst install) \
		&& ln -s build/lib.* LO_lib \
	)

endif

ifeq ($(OS),MACOSX)

python3_fw_prefix=$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO/LibreOfficePython.framework

# rule to allow relocating the whole framework, removing reference to buildinstallation directory
$(call gb_ExternalProject_get_state_target,python3,fixscripts) : $(call gb_ExternalProject_get_state_target,python3,build)
	$(call gb_Output_announce,python3 - remove reference to installroot from scripts,build,CUS,5)
	$(COMMAND_ECHO)for file in \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/2to3 \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/2to3-$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/idle$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/pydoc$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)-config \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m-config \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/pyvenv-$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) ; do \
	{ rm "$$file" && awk '\
		BEGIN {print "#!/bin/bash\n\
origpath=$$(pwd)\n\
bindir=$$(cd $$(dirname \"$$0\") ; pwd)\n\
cd \"$$origpath\"\n\
\"$$bindir/../Resources/Python.app/Contents/MacOS/LibreOfficePython\" - $$@ <<EOF"} \
		FNR==1{next} \
		      {print} \
		END   {print "EOF"}' > "$$file" ; } < "$$file" ; chmod +x "$$file" ; done
	touch $@

$(call gb_ExternalProject_get_state_target,python3,fixinstallnames) : $(call gb_ExternalProject_get_state_target,python3,build)
	$(INSTALL_NAME_TOOL) -change \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@executable_path/../../../../LibreOfficePython \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/Resources/Python.app/Contents/MacOS/LibreOfficePython
	touch $@

# also delete binaries that are symlinked in scp2
$(call gb_ExternalProject_get_state_target,python3,executables) : $(call gb_ExternalProject_get_state_target,python3,build)
	cd $(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin ; \
	for file in python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
	            python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m \
	            pythonw$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) ; do \
	$(INSTALL_NAME_TOOL) -change \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@executable_path/../LibreOfficePython $$file ; done
	touch $@

endif

# vim: set noet sw=4 ts=4:
