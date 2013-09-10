# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

gb_INSTROOT := $(INSTDIR)
gb_DEVINSTALLROOT := $(gb_INSTROOT)

# Convert path to file URL.
define gb_Helper_make_url
file:///$(strip $(1))
endef

# YaccTarget class

define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) --defines=$(4) -o $(5) $(1) && touch $(3) )

endef

# CObject class

# $(call gb_CObject__command,object,relative-source,source,dep-file)
define gb_CObject__command
$(call gb_Output_announce,$(2).c,$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	unset INCLUDE && \
	$(if $(filter YES,$(COBJECT_X64)), $(CXX_X64_BINARY), $(gb_CC)) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(T_CFLAGS) $(T_CFLAGS_APPEND) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		-Fd$(PDBFILE) \
		$(gb_COMPILERDEPFLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		$(if $(filter YES,$(COBJECT_X64)), -U_X86_ -D_AMD64_,) \
		-c $(3) \
		-Fo$(1)) $(call gb_create_deps,$(4),$(1),$(3))
endef


# CxxObject class

# $(call gb_CxxObject__command,object,relative-source,source,dep-file)
define gb_CxxObject__command
$(call gb_Output_announce,$(2).cxx,$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	unset INCLUDE && \
	$(if $(filter YES,$(CXXOBJECT_X64)), $(CXX_X64_BINARY), $(gb_CXX)) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(T_CXXFLAGS) $(T_CXXFLAGS_APPEND) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CXXFLAGS_WERROR)) \
		-Fd$(PDBFILE) \
		$(PCHFLAGS) \
		$(gb_COMPILERDEPFLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		$(if $(filter YES,$(CXXOBJECT_X64)), -U_X86_ -D_AMD64_,) \
		-c $(3) \
		-Fo$(1)) $(call gb_create_deps,$(4),$(1),$(3))
endef


# PrecompiledHeader class

# Note: MSVC has a race condition when dealing with .pdb files, that can result in error C1033 when
# the .pdb file already exists and two cl.exe invocations try to modify it at the same time (which
# is apparently most likely when generating both .pch files). The no-exceptions variant should be
# rarely needed now, but in case this turns out to be a problem in practice, this will need to
# be handled somehow (such as order-dependency of one on another).

gb_PrecompiledHeader_get_enableflags = -Yu$(1).hxx \
	-FI$(1).hxx \
	-Fp$(call gb_PrecompiledHeader_get_target,$(1)) \
	$(gb_PCHWARNINGS)

# MSVC PCH needs extra .obj created during the creation of the PCH file
gb_PrecompiledHeader_get_objectfile = $(1).obj

define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2))) && \
	unset INCLUDE && \
	$(gb_CXX) \
		$(4) $(5) -Fd$(PDBFILE) \
		$(gb_LTOFLAGS) \
		$(gb_COMPILERDEPFLAGS) \
		-I$(dir $(3)) \
		$(6) \
		-c $(3) \
		-Yc$(notdir $(patsubst %.cxx,%.hxx,$(3))) -Fp$(1) -Fo$(1).obj) $(call gb_create_deps,$(call gb_PrecompiledHeader_get_dep_target,$(2)),$(1),$(3))
endef

# AsmObject class

gb_AsmObject_get_source = $(1)/$(2).asm

define gb_AsmObject__command
$(call gb_Output_announce,$(2),$(true),ASM,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	"$(ML_EXE)" /c /Cp $(gb_AFLAGS) -D$(COM) /Fo$(1) $(3)) && \
	echo "$(1) : $(3)" > $(4)
endef


# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)

gb_LinkTarget_INCLUDE :=\
	$(subst -I. , ,$(SOLARINC)) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \
	-I$(BUILDDIR)/config_$(gb_Side) \

gb_LinkTarget_get_pdbfile = $(call gb_LinkTarget_get_target,)pdb/$(1).pdb

# avoid fatal error LNK1170 for Library_merged
define gb_LinkTarget_MergedResponseFile
cut -f -1000 -d ' ' $${RESPONSEFILE} > $${RESPONSEFILE}.1 && \
cut -f 1001- -d ' ' $${RESPONSEFILE} >> $${RESPONSEFILE}.1 && \
mv $${RESPONSEFILE}.1 $${RESPONSEFILE} &&
endef

ifeq ($(CPUNAME),X86_64)
MSC_SUBSYSTEM_VERSION=$(COMMA)5.02
else
MSC_SUBSYSTEM_VERSION=$(COMMA)5.01
endif

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100, \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),$(shell cat $(extraobjectlist))) \
		$(PCHOBJS) $(NATIVERES)) && \
		$(if $(filter $(call gb_Library_get_linktargetname,merged),$(2)),$(call gb_LinkTarget_MergedResponseFile)) \
	unset INCLUDE && \
	$(if $(filter YES,$(LIBRARY_X64)), $(LINK_X64_BINARY), $(gb_LINK)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter StaticLibrary,$(TARGETTYPE)),-LIB) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter YES,$(LIBRARY_X64)),,$(if $(filter YES,$(TARGETGUI)), -SUBSYSTEM:WINDOWS$(MSC_SUBSYSTEM_VERSION), -SUBSYSTEM:CONSOLE$(MSC_SUBSYSTEM_VERSION))) \
		$(if $(filter YES,$(LIBRARY_X64)), -MACHINE:X64) \
		$(if $(filter YES,$(LIBRARY_X64)), \
			-LIBPATH:$(OUTDIR)/lib/x64 \
			-LIBPATH:$(COMPATH)/lib/amd64 \
			-LIBPATH:$(WINDOWS_SDK_HOME)/lib/x64 \
		    $(if $(filter 80,$(WINDOWS_SDK_VERSION)),-LIBPATH:$(WINDOWS_SDK_HOME)/lib/win8/um/x64)) \
		$(T_LDFLAGS) \
		@$${RESPONSEFILE} \
		$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib))) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_filename,$(lib))) \
		$(LIBS) \
		$(if $(filter-out StaticLibrary,$(TARGETTYPE)),user32.lib) \
		$(if $(DLLTARGET),-out:$(DLLTARGET) -implib:$(1),-out:$(1)); RC=$$?; rm $${RESPONSEFILE} \
	$(if $(DLLTARGET),; if [ ! -f $(DLLTARGET) ]; then rm -f $(1) && false; fi) \
	$(if $(filter Library,$(TARGETTYPE)),&& if [ -f $(DLLTARGET).manifest ]; then mt.exe $(MTFLAGS) -nologo -manifest $(DLLTARGET).manifest -outputresource:$(DLLTARGET)\;2 && touch -r $(1) $(DLLTARGET).manifest $(DLLTARGET); fi) \
	$(if $(filter Executable,$(TARGETTYPE)),&& if [ -f $(1).manifest ]; then mt.exe $(MTFLAGS) -nologo -manifest $(1).manifest -outputresource:$(1)\;1 && touch -r $(1) $(1).manifest; fi) \
	$(if $(filter YES,$(TARGETGUI)),&& if [ -f $(SRCDIR)/solenv/inc/DeclareDPIAware.manifest ]; then mt.exe $(MTFLAGS) -nologo -manifest $(SRCDIR)/solenv/inc/DeclareDPIAware.manifest -updateresource:$(1)\;1 ; fi) \
	$(if $(filter Library,$(TARGETTYPE)),&& \
		echo $(notdir $(DLLTARGET)) > $(1).exports.tmp && \
		$(if $(filter YES,$(LIBRARY_X64)),$(LINK_X64_BINARY),$(gb_LINK)) \
			-dump -exports $(1) >> $(1).exports.tmp && \
		if cmp -s $(1).exports.tmp $(1).exports; \
			then rm $(1).exports.tmp; \
			else mv $(1).exports.tmp $(1).exports; touch -r $(1) $(1).exports; \
		fi) \
	; exit $$RC)
endef

define gb_MSVCRT_subst
$(if $(MSVC_USE_DEBUG_RUNTIME),$(subst msvcrt,msvcrtd,$(subst msvcprt,msvcprtd,$(subst libcmt,libcmtd,$(subst libcpmt,libcpmtd,$(subst msvcmrt,msvcmrtd,$(1)))))),$(1))
endef

define gb_LinkTarget_use_system_win32_libs
$(call gb_LinkTarget_add_libs,$(1),$(foreach lib,$(2),$(call gb_MSVCRT_subst,$(lib)).lib))
endef

# Flags common for PE executables (EXEs and DLLs) 
gb_Windows_PE_TARGETTYPEFLAGS := \
	-release \
	-opt:noref \
	-incremental:no \
	-debug \
	$(if $(filter NO,$(LIBRARY_X64)), -safeseh) \
	-nxcompat \
	-dynamicbase \
	-manifest

ifeq ($(ENABLE_LTO),TRUE)
gb_Windows_PE_TARGETTYPEFLAGS += -LTCG
endif

# Library class


gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := \
	-DLL \
	$(gb_Windows_PE_TARGETTYPEFLAGS)

gb_Library_get_rpath :=

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_KNOWNLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

gb_Library_DLLEXT := .dll
gb_Library_MAJORVER := 3
gb_Library_RTEXT := MSC$(gb_Library_DLLEXT)
gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \

# An assembly is a special kind of library for CLI
define gb_Library_Assembly
$(call gb_Library_Library,$(1))
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))) : NATIVERES :=

endef

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3).manifest \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
)

$(if $(filter $(gb_MERGEDLIBS),$(1)),,\
$(call gb_Library_add_auxtarget,$(1),$(OUTDIR)/bin/$(notdir $(3))))

# substitute .pyd here because pyuno has to follow python's crazy conventions
ifneq ($(ENABLE_CRASHDUMP),)
$(call gb_Library_add_auxtargets,$(1),\
	$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.pdb,$(patsubst %.pyd,%.dll,$(3)))) \
	$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.ilk,$(patsubst %.pyd,%.dll,$(3)))) \
)
else
$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.dll,%.pdb,$(patsubst %.pyd,%.dll,$(3))) \
	$(patsubst %.dll,%.ilk,$(patsubst %.pyd,%.dll,$(3))) \
)
endif

$(call gb_Library_add_default_nativeres,$(1),$(1)/default)

$(if $(filter $(gb_MERGEDLIBS),$(1)),,$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3),$(1)))

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(2))
$(call gb_WinResTarget_set_rcfile,$(2),include/default)
$(call gb_WinResTarget_add_defs,$(2),\
		-DVERVARIANT="$(LIBO_VERSION_PATCH)" \
		-DRES_APP_VENDOR="$(OOO_VENDOR)" \
		-DORG_NAME="$(call gb_Library_get_dllname,$(1))"\
		-DINTERNAL_NAME="$(subst $(gb_Library_DLLEXT),,$(call gb_Library_get_dllname,$(1)))" \
		-DADDITIONAL_VERINFO1="" \
		-DADDITIONAL_VERINFO2="" \
		-DADDITIONAL_VERINFO3="" \
)
$(call gb_Library_add_nativeres,$(1),$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_WinResTarget_get_clean_target,$(2))

endef

define gb_LinkTarget_add_nativeres
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_WinResTarget_get_target,$(2))
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES += $(call gb_WinResTarget_get_target,$(2))

endef

define gb_LinkTarget_set_nativeres
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_WinResTarget_get_target,$(2))
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES := $(call gb_WinResTarget_get_target,$(2))

endef

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef

# in effect this just causes the .export target to be touched
# cannot be the .lib itself because that causes attempts to get it linked :(
define gb_Library_get_exports_target
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))).exports
endef


# StaticLibrary class

gb_StaticLibrary_get_filename = $(1).lib
gb_StaticLibrary_PLAINEXT := .lib

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
)

endef

# Executable class

gb_Executable_EXT := .exe
gb_Executable_EXT_for_build := .exe
gb_Executable_TARGETTYPEFLAGS := $(gb_Windows_PE_TARGETTYPEFLAGS)

gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.exe,%.pdb,$(call gb_LinkTarget_get_target,$(2))) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
)

$(call gb_Executable_add_auxtarget,$(1),$(call gb_Executable_get_target,$(1)).manifest)
$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)).manifest,$(call gb_LinkTarget_get_target,$(2)).manifest,$(1))

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE := $(call gb_LinkTarget_get_pdbfile,$(2))
$(call gb_LinkTarget_get_target,$(2)) : TARGETGUI :=

endef

# CppunitTest class

gb_CppunitTest_DEFS := -D_DLL
# cppunittester.exe is in the cppunit subdirectory of ${OUTDIR}/bin,
# thus it won't find its DLLs unless ${OUTDIR}/bin is added to PATH.
gb_CppunitTest_CPPTESTPRECOMMAND := $(gb_Helper_set_ld_path)

gb_CppunitTest_SYSPRE := itest_
gb_CppunitTest_EXT := .lib
gb_CppunitTest_LIBDIR := $(gb_Helper_OUTDIRLIBDIR)
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = test_$(1).dll

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3) \
	$(3).manifest \
	$(patsubst %.dll,%.pdb,$(3)) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(patsubst %.dll,%.ilk,$(3)) \
)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

# JunitTest class

gb_defaultlangiso := en-US
gb_smoketest_instset := $(SRCDIR)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(gb_defaultlangiso)/OOo_*_install-arc_$(gb_defaultlangiso).zip

ifneq ($(OOO_TEST_SOFFICE),)
gb_JunitTest_SOFFICEARG:=$(OOO_TEST_SOFFICE)
else
ifneq ($(gb_JunitTest_DEBUGRUN),)
gb_JunitTest_SOFFICEARG:=connect:pipe,name=$(USER)
else
gb_JunitTest_SOFFICEARG:=path:$(gb_DEVINSTALLROOT)/program/soffice
endif
endif


define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:$(gb_DEVINSTALLROOT)/program/soffice.exe}" \
	-Dorg.openoffice.test.arg.env=PATH="$$$$PATH" \
	-Dorg.openoffice.test.arg.user=$(call gb_Helper_make_url,$(call gb_JunitTest_get_userdir,$(1)))

endef

# PythonTest class

gb_PythonTest_PRECOMMAND := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}$(gb_DEVINSTALLROOT)/URE/bin:$(gb_DEVINSTALLROOT)/program:$(OUTDIR)/bin

# SrsPartTarget class

ifeq ($(gb_FULLDEPS),$(true))
gb_SrsPartTarget__command_target = $(OUTDIR)/bin/makedepend$(gb_Executable_EXT)
define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(2) \
		-o .src \
		-p $(dir $(call gb_SrsPartTarget_get_target,$(1))) \
		-f $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef
else
gb_SrsPartTarget__command_target =
gb_SrsPartTarget__command_dep =
endif

# WinResTarget class

gb_WinResTarget_POSTFIX :=.res

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RC ,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		$(INCLUDE) \
		-Fo$(1) \
		$(RCFILE) )
endef

define gb_WinResTarget_WinResTarget_platform
$(call gb_WinResTarget_get_target,$(1)) : $(call gb_Package_get_target,solenv_inc)

endef

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
gb_WinResTarget__command_target = $(OUTDIR)/bin/makedepend$(gb_Executable_EXT)
define gb_WinResTarget__command_dep
$(call gb_Output_announce,RC:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(RCFILE) \
		-o .res \
		-p $(dir $(3)) \
		-f $(1))
endef
else
gb_WinResTarget__command_target =
gb_WinResTarget__command_dep =
endif

# InstallModuleTarget class

define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=MSC \
)

endef

# ExternalProject class

# Use the gcc wrappers for a autoconf based project
#
# gb_ExternalProject_register_targets project state_target
define gb_ExternalProject_use_autoconf
$(call gb_ExternalProject_get_preparation_target,$(1)) : $(call gb_Executable_get_runtime_dependencies,gcc-wrapper) \
	$(call gb_Executable_get_runtime_dependencies,g++-wrapper)
$(call gb_ExternalProject_get_state_target,$(1),$(2)): WRAPPERS := $(gb_AUTOCONF_WRAPPERS)
endef

# if ccache is enabled, then split it and use lastword as REAL_FOO
# /opt/lo/bin/ccache /cygdrive/c/PROGRA~2/MICROS~2.0/VC/bin/cl.exe

gb_AUTOCONF_WRAPPERS = \
	REAL_CC="$(shell cygpath -w $(CC))" \
	CC="$(call gb_Executable_get_target,gcc-wrapper)" \
	REAL_CXX="$(shell cygpath -w  $(CXX))" \
	CXX="$(call gb_Executable_get_target,g++-wrapper)" \
    LD="$(shell cygpath -w $(COMPATH)/bin/link.exe) -nologo"

# InstallScript class

gb_InstallScript_EXT := .inf

# CliAssemblyTarget class

gb_CliAssemblyTarget_POLICYEXT := $(gb_Library_DLLEXT)
gb_CliAssemblyTarget_get_dll = $(OUTDIR)/bin/$(1)$(gb_CliAssemblyTarget_POLICYEXT)

# Extension class

gb_Extension_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/license.txt

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(shell cygpath -u $(TARFILE_LOCATION))

# UnoApiHeadersTarget class

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

# Package class
gb_Package_SDKDIRNAME := sdk

# UIMenubarTarget class

define gb_UIMenubarTarget__command
$(call gb_Output_announce,$(2),$(true),UIM,1)
cp $(3) $(1)

endef

gb_UIMenubarTarget_UIMenubarTarget_platform :=

# Python
gb_Python_PRECOMMAND := PATH="$(shell cygpath -w $(INSTDIR)/program);$(shell cygpath -w $(OUTDIR)/bin)" PYTHONHOME="$(INSTDIR)/program/python-core-$(PYTHON_VERSION)" PYTHONPATH="$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib;$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib/lib-dynload:$(INSTDIR)/program"
gb_Python_INSTALLED_EXECUTABLE := $(gb_DEVINSTALLROOT)/program/python.exe

# vim: set noet sw=4:
