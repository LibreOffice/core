# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

gb_Executable_EXT:=

include $(GBUILDDIR)/platform/com_GCC_defs.mk

gb_CCVER := $(shell $(gb_CC) -dumpversion | $(gb_AWK) -F. -- '{ print $$1*10000+$$2*100+$$3 }')
gb_GccLess460 := $(shell expr $(gb_CCVER) \< 40600)

gb_MKTEMP := mktemp -t gbuild.XXXXXX

ifneq ($(origin AR),default)
gb_AR := $(AR)
endif

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	$(PTHREAD_CFLAGS) \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_CFLAGS := \
	$(gb_CFLAGS_COMMON) \
	-fPIC \
	-Wdeclaration-after-statement \
	-Wshadow \

gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-fPIC \
	-Wshadow \
	-Wsign-promo \
	-Woverloaded-virtual \

ifneq ($(COM_GCC_IS_CLANG),TRUE)
# Only GCC 4.6 has a fix for <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=7302>
# "-Wnon-virtual-dtor should't complain of protected dtor" and supports #pragma
# GCC diagnostic push/pop required e.g. in cppuhelper/propertysetmixin.hxx to
# silence warnings about a protected, non-virtual dtor in a class with virtual
# members and friends:
ifeq ($(gb_GccLess460),1)
gb_CXXFLAGS += -Wno-non-virtual-dtor
else
gb_CXXFLAGS += -Wnon-virtual-dtor
endif
else
gb_CXXFLAGS += -Wnon-virtual-dtor
endif

ifeq ($(HAVE_SFINAE_ANONYMOUS_BROKEN),TRUE)
gb_COMPILERDEFS += \
        -DHAVE_SFINAE_ANONYMOUS_BROKEN \

endif

# enable debug STL
ifeq ($(gb_PRODUCT),$(false))
gb_COMPILERDEFS += \
	-D_GLIBCXX_DEBUG \

endif

#At least SLED 10.2 gcc 4.3 overly agressively optimizes uno::Sequence into
#junk, so only strict-alias on >= 4.6.0
gb_StrictAliasingUnsafe := $(gb_GccLess460)

ifeq ($(gb_StrictAliasingUnsafe),1)
gb_CFLAGS += -fno-strict-aliasing
gb_CXXFLAGS += -fno-strict-aliasing
endif

ifeq ($(HAVE_CXX0X),TRUE)
#Currently, as well as for its own merits, c++11/c++0x mode allows use to use
#a template for SAL_N_ELEMENTS to detect at compiler time its misuse
gb_CXXFLAGS += -std=gnu++0x

#We have so many std::auto_ptr uses that we need to be able to disable
#warnings for those so that -Werror continues to be useful, seeing as moving
#to unique_ptr isn't an option when we must support different compilers

#When we are using 4.6.0 we can use gcc pragmas to selectively silence auto_ptr
#warnings in isolation, but for <= 4.5.X we need to globally disable
#deprecation
ifeq ($(gb_GccLess460),1)
gb_CXXFLAGS += -Wno-deprecated-declarations
endif
endif

ifeq ($(ENABLE_LTO),TRUE)
ifneq ($(COM_GCC_IS_CLANG),TRUE)
gb_LinkTarget_LDFLAGS += -fuse-linker-plugin $(gb_COMPILERDEFAULTOPTFLAGS)
endif
endif

ifneq ($(strip $(SYSBASE)),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
gb_LinkTarget_LDFLAGS += \
	-Wl,--sysroot=$(SYSBASE)
endif

gb_LinkTarget_LDFLAGS += \
	-Wl,-rpath-link,$(SYSBASE)/lib:$(SYSBASE)/usr/lib \
	-Wl,-z,combreloc \
	$(subst -L../lib , ,$(SOLARLIB)) \

ifeq ($(HAVE_LD_HASH_STYLE),TRUE)
gb_LinkTarget_LDFLAGS += \
	-Wl,--hash-style=$(WITH_LINKER_HASH_STYLE) \

endif

ifneq ($(HAVE_LD_BSYMBOLIC_FUNCTIONS),)
gb_LinkTarget_LDFLAGS += \
	-Wl,--dynamic-list-cpp-new \
	-Wl,--dynamic-list-cpp-typeinfo \
	-Wl,-Bsymbolic-functions \

endif

ifneq ($(gb_DEBUGLEVEL),0)
gb_LINKEROPTFLAGS :=
else
gb_LINKEROPTFLAGS := -Wl,-O1
endif

gb_COMPILEROPTFLAGS := $(gb_COMPILERDEFAULTOPTFLAGS)
gb_COMPILERNOOPTFLAGS := -O0

# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef

gb_LinkTarget__RPATHS := \
	URELIB:\dORIGIN \
	UREBIN:\dORIGIN/../lib:\dORIGIN \
	OOO:\dORIGIN:\dORIGIN/../ure-link/lib \
	SDKBIN:\dORIGIN/../../ure-link/lib \
	OXT: \
	NONE:\dORIGIN/../lib:\dORIGIN \

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)

ifeq ($(gb_SYMBOL),$(true))
gb_LinkTarget_CXXFLAGS += $(GGDB2)
gb_LinkTarget_CFLAGS += $(GGDB2)
endif

# note that `cat $(extraobjectlist)` is needed to build with older gcc versions, e.g. 4.1.2 on SLED10
# we want to use @$(extraobjectlist) in the long run
define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_CXX) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(if $(SOVERSION),-Wl$(COMMA)--soname=$(notdir $(1)).$(SOVERSION)) \
		$(if $(SOVERSIONSCRIPT),-Wl$(COMMA)--version-script=$(SOVERSIONSCRIPT))\
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),`cat $(extraobjectlist)`) \
		-Wl$(COMMA)--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) -Wl$(COMMA)--end-group \
		-Wl$(COMMA)--no-as-needed \
		$(LIBS) \
		$(patsubst lib%.a,-l%,$(patsubst lib%.so,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib))))) \
		-o $(if $(SOVERSION),$(1).$(SOVERSION),$(1)))
	$(if $(SOVERSION),ln -sf $(notdir $(1)).$(SOVERSION) $(1))
endef

define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Library CppunitTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -shared -Wl,-z,noexecstack
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .so
gb_Library_DLLEXT := .so
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)

gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)

gb_Library_PLAINLIBS_NONE += \
	dl \
	GL \
	GLU \
	ICE \
	m \
	pthread \
	SM \
	ICE \
	X11 \
	Xext \
	Xrender \

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

define gb_Library__get_rpath
$(if $(1),$(strip -Wl,-z,origin '-Wl,-rpath,$(1)' '-Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)'))
endef

define gb_Library_get_rpath
$(call gb_Library__get_rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library_get_rpath,$(1))

endef

gb_Library__set_soversion_script_platform = $(gb_Library__set_soversion_script)


# StaticLibrary class

gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_StaticLibrary_platform =


# Executable class

gb_Executable_EXT :=

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \


define gb_Executable_get_rpath
-Wl,-z,origin '-Wl,-rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1)))' \
-Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)
endef

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Executable_get_rpath,$(1))

endef


# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := \
    $(call gb_Helper_extend_ld_path,$(OUTDIR_FOR_BUILD)/lib/sqlite)
gb_CppunitTest_SYSPRE := libtest_
gb_CppunitTest_EXT := .so
gb_CppunitTest_LIBDIR := $(gb_Helper_OUTDIRLIBDIR)
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = $(gb_CppunitTest_get_filename)

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=

endef

# JunitTest class

ifneq ($(OOO_TEST_SOFFICE),)
gb_JunitTest_SOFFICEARG:=$(OOO_TEST_SOFFICE) 
else
ifneq ($(gb_JunitTest_DEBUGRUN),)
gb_JunitTest_SOFFICEARG:=connect:pipe,name=$(USER)
else
gb_JunitTest_SOFFICEARG:=path:$(DEVINSTALLDIR)/opt/program/soffice
endif
endif

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
	-Dorg.openoffice.test.arg.user=file://$(call gb_JunitTest_get_userdir,$(1)) \
	-Dorg.openoffice.test.arg.workdir=$(call gb_JunitTest_get_userdir,$(1)) \
	-Dorg.openoffice.test.arg.postprocesscommand=$(GBUILDDIR)/platform/unxgcc_gdbforjunit.sh \
	-Dorg.openoffice.test.arg.soffice="$(gb_JunitTest_SOFFICEARG)" \

endef

# Module class

define gb_Module_DEBUGRUNCOMMAND
OFFICESCRIPT=`mktemp` && \
printf 'if [ -e $(DEVINSTALLDIR)/opt/program/ooenv ]; then . $(DEVINSTALLDIR)/opt/program/ooenv; fi\n' > $${OFFICESCRIPT} && \
printf "gdb $(DEVINSTALLDIR)/opt/program/soffice.bin" >> $${OFFICESCRIPT} && \
printf " -ex \"set args --norestore --nologo '--accept=pipe,name=$(USER);urp;' -env:UserInstallation=file://$(DEVINSTALLDIR)/\"" >> $${OFFICESCRIPT} && \
$(SHELL) $${OFFICESCRIPT} && \
rm $${OFFICESCRIPT}
endef

# InstallModuleTarget class

# NOTE: values of SHORTSTDC3 and SHORTSTDCPP3 are hardcoded, because we
# do not really need all the variability of definition that was
# originally in solenv/inc/tg_compv.mk . Each of the macros is only used
# on one place, and only for Linux.
define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=gcc3 \
	-DCOMNAME=gcc3 \
	-DSHORTSTDC3=1 \
	-DSHORTSTDCPP3=6 \
	-D_gcc3 \
	$(if $(filter TRUE,$(SOLAR_JAVA)),-DSOLAR_JAVA) \
)

endef

# ScpConvertTarget class

gb_ScpConvertTarget_ScpConvertTarget_platform :=

# InstallScript class

gb_InstallScript_EXT := .ins

# CliAssemblyTarget class

gb_CliAssemblyTarget_POLICYEXT :=
gb_CliAssemblyTarget_get_dll :=

# ExtensionTarget class

gb_ExtensionTarget_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/LICENSE

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

# Python
gb_PYTHON_PRECOMMAND := $(gb_Helper_set_ld_path) PYTHONHOME=$(OUTDIR)/lib/python PYTHONPATH=$(OUTDIR)/lib/python:$(OUTDIR)/lib/python/lib-dynload

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
