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
		removeunnecessarystuff \
	) \
))

ifeq ($(OS),WNT)

# TODO: using Debug configuration and related mangling of pyconfig.h

# at least for MSVC 2008 it is necessary to clear MAKEFLAGS because
# nmake is invoked
$(call gb_ExternalProject_get_state_target,python3,build) :
	$(call gb_ExternalProject_run,build,\
		MAKEFLAGS= MSBuild.exe pcbuild.sln /t:Build \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			/p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			/maxcpucount \
			$(if $(filter 150,$(VCVER)),/p:PlatformToolset=v141 /p:VisualStudioVersion=15.0 /ToolsVersion:15.0) \
			$(if $(filter 150-10,$(VCVER)-$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
	,PCBuild)

else

# this was added in 2004, hopefully is obsolete now (and why only intel anyway)? $(if $(filter SOLARIS-INTEL,$(OS)$(CPUNAME)),--disable-ipv6)

# --with-system-expat: this should find the one in the solver (or system)

# create a symlink "LO_lib" because the .so are in a directory with platform
# specific name like build/lib.linux-x86_64-3.3

python3_cflags = $(ZLIB_CFLAGS)
ifneq (,$(ENABLE_VALGRIND))
    python3_cflags += $(VALGRIND_CFLAGS)
endif

$(call gb_ExternalProject_get_state_target,python3,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter MACOSX,$(OS)), \
			$(if $(filter 10.8 10.9 10.10 10.11,$(MACOSX_DEPLOYMENT_TARGET)), \
				ac_cv_func_getentropy=no \
				ac_cv_func_clock_gettime=no \
			) \
		) \
		./configure \
		$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(ENABLE_VALGRIND),--with-valgrind) \
		--prefix=/python-inst \
		$(if $(filter MACOSX,$(OS)),,--with-system-expat) \
		$(if $(filter AIX,$(OS)), \
			--disable-ipv6 --with-threads OPT="-g0 -fwrapv -O3 -Wall", \
			$(if $(gb_Module_CURRENTMODULE_DEBUG_ENABLED), \
				OPT="$(gb_COMPILERNOOPTFLAGS) $(gb_DEBUGINFO_FLAGS)")) \
		$(if $(filter MACOSX,$(OS)), \
			$(if $(filter INTEL,$(CPUNAME)),--enable-universalsdk=$(MACOSX_SDK_PATH) \
                                --with-universal-archs=intel \
            ) \
			--enable-framework=/@__________________________________________________OOO --with-framework-name=LibreOfficePython, \
			--enable-shared \
		) \
		CC="$(strip $(CC) \
			$(if $(SYSTEM_OPENSSL),,-I$(call gb_UnpackedTarball_get_dir,openssl)/include \
				$(if $(DISABLE_OPENSSL),,-I$(call gb_UnpackedTarball_get_dir,openssl)/include)) \
			$(if $(SYSTEM_EXPAT),,-I$(call gb_UnpackedTarball_get_dir,expat)/lib) \
			$(if $(SYSBASE), -I$(SYSBASE)/usr/include) \
			)" \
		$(if $(python3_cflags),CFLAGS='$(python3_cflags)') \
		$(if $(filter -fsanitize=%,$(CC)),LINKCC="$(CXX) -pthread") \
		LDFLAGS="$(strip $(LDFLAGS) \
			$(if $(SYSTEM_OPENSSL),,-L$(call gb_UnpackedTarball_get_dir,openssl)) \
			$(if $(SYSTEM_EXPAT),,-L$(gb_StaticLibrary_WORKDIR)) \
			$(if $(SYSTEM_ZLIB),,-L$(gb_StaticLibrary_WORKDIR)) \
			$(if $(SYSBASE), -L$(SYSBASE)/usr/lib) \
			$(gb_LTOFLAGS) \
			)" \
		&& MAKEFLAGS= $(MAKE) \
			$(if $(filter MACOSX,$(OS)),DESTDIR=$(EXTERNAL_WORKDIR)/python-inst install) \
			$(if $(SYSTEM_ZLIB),,ZLIB_INCDIR=$(WORKDIR)/UnpackedTarball/zlib) \
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
	{ rm "$$file" && $(gb_AWK) '\
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
	for file in $(shell $(FIND) $(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/lib-dynload -name "*.so") ; do \
	$(INSTALL_NAME_TOOL) -change \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@loader_path/../../../LibreOfficePython $$file ; done
	touch $@

# also delete binaries that are symlinked in scp2
$(call gb_ExternalProject_get_state_target,python3,executables) : $(call gb_ExternalProject_get_state_target,python3,build)
	cd $(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin ; \
	for file in python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
	            python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m ; do \
	$(INSTALL_NAME_TOOL) -change \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@executable_path/../LibreOfficePython $$file ; done
	touch $@

$(call gb_ExternalProject_get_state_target,python3,removeunnecessarystuff) : $(call gb_ExternalProject_get_state_target,python3,build)
	$(call gb_Output_announce,python3 - remove the stuff we don't need to ship,build,CUS,5)
	rm -rf $(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/test

endif

# vim: set noet sw=4 ts=4:
