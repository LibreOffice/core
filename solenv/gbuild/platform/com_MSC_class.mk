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

# $(call gb_CObject__command_pattern,object,flags,source,dep-file,compiler-plugins)
define gb_CObject__command_pattern
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	unset INCLUDE && \
	$(if $(filter YES,$(CXXOBJECT_X64)), $(CXX_X64_BINARY), \
		$(if $(filter YES,$(PE_X86)), $(CXX_X86_BINARY), \
			$(if $(filter %.c,$(3)), $(gb_CC), \
				$(if $(filter -clr,$(2)), \
					$(MSVC_CXX) -I$(SRCDIR)/solenv/clang-cl,$(gb_CXX))))) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(2) \
		$(if $(EXTERNAL_CODE), \
			$(if $(filter -clr,$(2)),,$(if $(COM_IS_CLANG),-Wno-undef)), \
			$(gb_DEFS_INTERNAL)) \
		$(if $(WARNINGS_NOT_ERRORS),$(if $(ENABLE_WERROR),$(if $(PLUGIN_WARNINGS_AS_ERRORS),$(gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS))),$(gb_CFLAGS_WERROR)) \
		$(if $(filter -clr,$(2)),,$(if $(5),$(gb_COMPILER_PLUGINS))) \
		$(if $(COMPILER_TEST),-fsyntax-only -ferror-limit=0 -Xclang -verify) \
		-Fd$(PDBFILE) \
		$(PCHFLAGS) \
		$(if $(COMPILER_TEST),,$(gb_COMPILERDEPFLAGS)) \
		$(INCLUDE) \
		$(if $(filter YES,$(CXXOBJECT_X64)), -U_X86_ -D_AMD64_,) \
		$(if $(filter YES,$(PE_X86)), -D_X86_ -U_AMD64_,) \
		-c $(3) \
		-Fo$(1)) $(if $(filter $(true),$(gb_SYMBOL)),/link /DEBUG:FASTLINK) \
		$(if $(COMPILER_TEST),,$(call gb_create_deps,$(4),$(1),$(3)))
endef

# PrecompiledHeader class

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
		$(if $(EXTERNAL_CODE),$(if $(COM_IS_CLANG),-Wno-undef),$(gb_DEFS_INTERNAL)) \
		$(gb_LTOFLAGS) \
		$(gb_COMPILERDEPFLAGS) \
		$(6) \
		-c $(3) \
		-Yc$(notdir $(patsubst %.cxx,%.hxx,$(3))) -Fp$(1) -Fo$(1).obj) $(call gb_create_deps,$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2)),$(1),$(3))
endef

# AsmObject class

gb_AsmObject_get_source = $(1)/$(2).asm

define gb_AsmObject__command
$(call gb_Output_announce,$(2),$(true),ASM,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	"$(ML_EXE)" /safeseh /c /Cp $(gb_AFLAGS) -D$(COM) /Fo$(1) $(3)) && \
	echo "$(1) : $(3)" > $(4)
endef


# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)
gb_LinkTarget_CXXCLRFLAGS := $(gb_CXXCLRFLAGS)

gb_LinkTarget_INCLUDE :=\
	$(SOLARINC) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \
	-I$(BUILDDIR)/config_$(gb_Side) \

# We must name the .pdb like libname.pdb, not libname.\(dll\|exe\|pyd\).pdb,
# otherwise WinDbg does not find it.
define gb_LinkTarget__get_pdb_filename
$(patsubst %.dll,%.pdb,$(patsubst %.exe,%.pdb,$(patsubst %.pyd,%.pdb,$(1))))
endef

gb_LinkTarget_get_pdbfile_in = \
 $(WORKDIR)/LinkTarget/$(call gb_LinkTarget__get_workdir_linktargetname,$(1)).objects.pdb

gb_LinkTarget_get_pdbfile_out = \
 $(call gb_LinkTarget__get_pdb_filename,$(WORKDIR)/LinkTarget/$(call gb_LinkTarget__get_workdir_linktargetname,$(1)))

gb_LinkTarget_get_ilkfile = \
 $(WORKDIR)/LinkTarget/$(call gb_LinkTarget__get_workdir_linktargetname,$(1)).ilk

gb_LinkTarget_get_manifestfile = \
 $(WORKDIR)/LinkTarget/$(call gb_LinkTarget__get_workdir_linktargetname,$(1)).manifest

gb_LinkTarget_get_linksearchpath_for_layer = \
	-LIBPATH:$(WORKDIR)/LinkTarget/StaticLibrary \
	-LIBPATH:$(INSTDIR)/$(SDKDIRNAME)/lib \
	$(if $(filter OXT,$(1)),\
		-LIBPATH:$(WORKDIR)/LinkTarget/ExtensionLibrary, \
		-LIBPATH:$(WORKDIR)/LinkTarget/Library)

# avoid fatal error LNK1170 for Library_merged
define gb_LinkTarget_MergedResponseFile
cat $${RESPONSEFILE} | sed 's/ /\n/g' | grep -v '^$$' > $${RESPONSEFILE}.1 && \
mv $${RESPONSEFILE}.1 $${RESPONSEFILE} &&
endef

ifeq ($(CPUNAME),X86_64)
MSC_SUBSYSTEM_VERSION=$(COMMA)5.02
else
MSC_SUBSYSTEM_VERSION=$(COMMA)5.01
endif

# the sort on the libraries is used to filter out duplicates to keep commandline
# length in check - otherwise the dupes easily hit the limit when linking mergedlib
define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(1) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100, \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(CXXCLROBJECTS),$(call gb_CxxClrObject_get_target,$(object))) \
		$(foreach object,$(GENCXXCLROBJECTS),$(call gb_GenCxxClrObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),$(shell cat $(extraobjectlist))) \
		$(PCHOBJS) $(NATIVERES)) && \
		$(if $(filter $(call gb_Library__get_workdir_linktargetname,merged),$(2)),$(call gb_LinkTarget_MergedResponseFile)) \
	unset INCLUDE && \
	$(if $(filter YES,$(LIBRARY_X64)), $(LINK_X64_BINARY), $(gb_LINK)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter StaticLibrary,$(TARGETTYPE)),-LIB) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter YES,$(LIBRARY_X64)),,$(if $(filter YES,$(TARGETGUI)), -SUBSYSTEM:WINDOWS$(MSC_SUBSYSTEM_VERSION), -SUBSYSTEM:CONSOLE$(MSC_SUBSYSTEM_VERSION))) \
		$(if $(filter YES,$(LIBRARY_X64)), -MACHINE:X64) \
		$(if $(filter YES,$(PE_X86)), -MACHINE:X86) \
		$(if $(filter YES,$(LIBRARY_X64)), \
			-LIBPATH:$(COMPATH)/lib/x64 \
			-LIBPATH:$(WINDOWS_SDK_HOME)/lib/x64 \
			-LIBPATH:$(UCRTSDKDIR)lib/$(UCRTVERSION)/ucrt/x64 \
		    $(if $(filter 80 81 10,$(WINDOWS_SDK_VERSION)),-LIBPATH:$(WINDOWS_SDK_HOME)/lib/$(WINDOWS_SDK_LIB_SUBDIR)/um/x64)) \
		$(if $(filter YES,$(PE_X86)), \
			-LIBPATH:$(COMPATH)/lib/x86 \
			-LIBPATH:$(WINDOWS_SDK_HOME)/lib/x86 \
			-LIBPATH:$(UCRTSDKDIR)lib/$(UCRTVERSION)/ucrt/x86 \
			$(if $(filter 80 81 10,$(WINDOWS_SDK_VERSION)),-LIBPATH:$(WINDOWS_SDK_HOME)/lib/$(WINDOWS_SDK_LIB_SUBDIR)/um/x86)) \
		$(T_LDFLAGS) \
		$(if $(filter Library CppunitTest Executable,$(TARGETTYPE)),/NATVIS:$(SRCDIR)/solenv/vs/LibreOffice.natvis) \
		@$${RESPONSEFILE} \
		$(foreach lib,$(sort $(LINKED_LIBS)),$(call gb_Library_get_ilibfilename,$(lib))) \
		$(foreach lib,$(sort $(LINKED_STATIC_LIBS)),$(call gb_StaticLibrary_get_filename,$(lib))) \
		$(if $(filter-out StaticLibrary,$(TARGETTYPE)),\
			$(sort $(T_LIBS)) user32.lib \
			-manifestfile:$(WORKDIR)/LinkTarget/$(2).manifest \
			-pdb:$(call gb_LinkTarget__get_pdb_filename,$(WORKDIR)/LinkTarget/$(2))) \
		$(if $(ILIBTARGET),-out:$(1) -implib:$(ILIBTARGET),-out:$(1)); RC=$$?; rm $${RESPONSEFILE} \
	$(if $(filter Library,$(TARGETTYPE)),; if [ ! -f $(ILIBTARGET) ]; then rm -f $(1); exit 42; fi) \
	$(if $(filter Library,$(TARGETTYPE)),&& if [ -f $(WORKDIR)/LinkTarget/$(2).manifest ]; then mt.exe $(MTFLAGS) -nologo -manifest $(WORKDIR)/LinkTarget/$(2).manifest $(SRCDIR)/solenv/gbuild/platform/win_compatibility.manifest -outputresource:$(1)\;2 && touch -r $(1) $(WORKDIR)/LinkTarget/$(2).manifest $(ILIBTARGET); fi) \
	$(if $(filter Executable,$(TARGETTYPE)),&& if [ -f $(WORKDIR)/LinkTarget/$(2).manifest ]; then mt.exe $(MTFLAGS) -nologo -manifest $(WORKDIR)/LinkTarget/$(2).manifest $(SRCDIR)/solenv/gbuild/platform/win_compatibility.manifest -outputresource:$(1)\;1 && touch -r $(1) $(WORKDIR)/LinkTarget/$(2).manifest; fi) \
	$(if $(filter Executable,$(TARGETTYPE)),&& mt.exe $(MTFLAGS) -nologo -manifest $(SRCDIR)/solenv/gbuild/platform/DeclareDPIAware.manifest -updateresource:$(1)\;1 ) \
	$(if $(filter Library,$(TARGETTYPE)),&& \
		echo $(notdir $(1)) > $(WORKDIR)/LinkTarget/$(2).exports.tmp && \
		$(if $(filter YES,$(LIBRARY_X64)),$(LINK_X64_BINARY),$(gb_LINK)) \
			-dump -exports $(ILIBTARGET) \
			>> $(WORKDIR)/LinkTarget/$(2).exports.tmp && \
		$(call gb_Helper_replace_if_different_and_touch,$(WORKDIR)/LinkTarget/$(2).exports.tmp,$(WORKDIR)/LinkTarget/$(2).exports,$(1))) \
	; exit $$RC)
endef

define gb_MSVCRT_subst
$(if $(MSVC_USE_DEBUG_RUNTIME),$(subst msvcrt,msvcrtd,$(subst msvcprt,msvcprtd,$(subst libcmt,libcmtd,$(subst libvcruntime,libvcruntimed,$(subst libucrt,libucrtd,$(subst libcpmt,libcpmtd,$(subst msvcmrt,msvcmrtd,$(1)))))))),$(1))
endef

define gb_LinkTarget_use_system_win32_libs
$(call gb_LinkTarget_add_libs,$(1),$(foreach lib,$(2),$(call gb_MSVCRT_subst,$(lib)).lib))
$(if $(call gb_LinkTarget__is_merged,$(1)),\
	$(call gb_LinkTarget_add_libs,$(call gb_Library_get_linktarget,merged),$(foreach lib,$(2),$(call gb_MSVCRT_subst,$(lib)).lib)))
endef

# Flags common for PE executables (EXEs and DLLs)
gb_Windows_PE_TARGETTYPEFLAGS := \
	-release \
	-opt:noref \
	$(if $(filter $(true),$(gb_SYMBOL)),-debug) \
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
	$(foreach lib,$(gb_Library_PLAINLIBS_OXT),$(lib):OXT) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \
	$(foreach lib,$(gb_Library_PLAINLIBS_SHLXTHDL),$(lib):SHLXTHDL) \

gb_Library_ILIBFILENAMES :=\
	$(foreach lib,$(gb_Library_KNOWNLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

gb_Library_DLLEXT := .dll
gb_Library_UDK_MAJORVER := 3
gb_Library_RTEXT := MSC$(gb_Library_DLLEXT)
gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_RTEXT)

gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OXT),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_SHLXTHDL),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \

# An assembly is a special kind of library for CLI
define gb_Library_Assembly
$(call gb_Library_Library,$(1))
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,$(1))) : NATIVERES :=

endef

define gb_Library_Library_platform
$(call gb_LinkTarget_set_ilibtarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(3)) \
	$(call gb_LinkTarget_get_manifestfile,$(2)) \
	$(call gb_LinkTarget_get_pdbfile_in,$(2)) \
	$(call gb_LinkTarget_get_pdbfile_out,$(2)) \
	$(call gb_LinkTarget_get_ilkfile,$(2)) \
)

$(call gb_Library_add_default_nativeres,$(1),$(1)/default)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile_in,$(2))

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(2))
$(call gb_WinResTarget_set_rcfile,$(2),include/default)
$(call gb_WinResTarget_add_defs,$(2),\
		-DVERVARIANT="$(LIBO_VERSION_PATCH)" \
		-DRES_APP_VENDOR="$(OOO_VENDOR)" \
		-DORG_NAME="$(call gb_Library_get_filename,$(1))"\
		-DINTERNAL_NAME="$(subst $(gb_Library_DLLEXT),,$(call gb_Library_get_filename,$(1)))" \
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

define gb_Library_get_ilibfilename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_ILIBFILENAMES)))
endef

gb_Library_get_sdk_link_dir = $(INSTDIR)/$(SDKDIRNAME)/lib

gb_Library_get_sdk_link_lib = $(gb_Library_get_ilib_target)

# StaticLibrary class

gb_StaticLibrary_get_filename = $(1).lib
gb_StaticLibrary_PLAINEXT := .lib

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile_in,$(2))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(call gb_LinkTarget_get_pdbfile_in,$(2)) \
)

endef

# Executable class

gb_Executable_EXT := .exe
gb_Executable_EXT_for_build := .exe
gb_Executable_TARGETTYPEFLAGS := $(gb_Windows_PE_TARGETTYPEFLAGS)

gb_Executable_get_rpath :=

# surprisingly some executables have exports so link.exe creates import lib
define gb_Executable_Executable_platform
$(call gb_LinkTarget_set_ilibtarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(3)) \
	$(call gb_LinkTarget_get_pdbfile_out,$(2)) \
	$(call gb_LinkTarget_get_pdbfile_in,$(2)) \
	$(call gb_LinkTarget_get_manifestfile,$(2)) \
)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE := $(call gb_LinkTarget_get_pdbfile_in,$(2))

endef

# CppunitTest class

gb_CppunitTest_UNITTESTFAILED := $(GBUILDDIR)/platform/unittest-failed-WNT.sh
gb_CppunitTest_PYTHONDEPS := $(call gb_Package_get_target,python3)
gb_CppunitTest_DEFS := -D_DLL
ifeq ($(GNUMAKE_WIN_NATIVE),TRUE)
gb_CppunitTest_CPPTESTPRECOMMAND := $(call gb_Helper_prepend_ld_path,$(shell cygpath -w $(gb_Library_DLLDIR));$(shell cygpath -w $(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/$(if $(MSVC_USE_DEBUG_RUNTIME),DebugDll,ReleaseDll)))
else
gb_CppunitTest_CPPTESTPRECOMMAND := $(call gb_Helper_prepend_ld_path,$(shell cygpath -u $(gb_Library_DLLDIR)):$(shell cygpath -u $(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/$(if $(MSVC_USE_DEBUG_RUNTIME),DebugDll,ReleaseDll)))
endif
gb_CppunitTest_get_filename = test_$(1).dll
gb_CppunitTest_get_ilibfilename = itest_$(1).lib

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_ilibtarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(3)) \
	$(3) \
	$(call gb_LinkTarget_get_manifestfile,$(2)) \
	$(call gb_LinkTarget_get_pdbfile_out,$(2)) \
	$(call gb_LinkTarget_get_pdbfile_in,$(2)) \
	$(call gb_LinkTarget_get_ilkfile,$(2)) \
)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile_in,$(2))

endef

# JunitTest class

ifneq ($(OOO_TEST_SOFFICE),)
gb_JunitTest_SOFFICEARG:=$(OOO_TEST_SOFFICE)
else
ifneq ($(gb_JunitTest_DEBUGRUN),)
gb_JunitTest_SOFFICEARG:=connect:pipe,name=$(USER)
else
gb_JunitTest_SOFFICEARG:=path:$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice
endif
endif


define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$(gb_JunitTest_SOFFICEARG)" \
	-Dorg.openoffice.test.arg.env=PATH="$$$$PATH" \
	-Dorg.openoffice.test.arg.user=$(call gb_Helper_make_url,$(call gb_JunitTest_get_userdir,$(1)))
	-Dorg.openoffice.test.arg.workdir=$(call gb_JunitTest_get_userdir,$(1)) \

endef


define gb_Module_DEBUGRUNCOMMAND
printf "\nAttach the debugger to soffice.bin\n\n"
OFFICESCRIPT=`mktemp` && \
printf "$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice.exe" > $${OFFICESCRIPT} && \
printf " --norestore --nologo '--accept=pipe,name=$(USER);urp;'\n" >> $${OFFICESCRIPT} && \
$(SHELL) $${OFFICESCRIPT} && \
rm $${OFFICESCRIPT}
endef




# PythonTest class

gb_PythonTest_PRECOMMAND := $(gb_CppunitTest_CPPTESTPRECOMMAND)
gb_PythonTest_DEPS = $(call gb_Package_get_target,python3) $(call gb_Executable_get_target,python)

ifeq ($(strip $(CPPUNITTRACE)),TRUE)
gb_CppunitTest_GDBTRACE := '$(DEVENV)' /debugexe
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

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
# FIXME this is used before TargetLocations is read?
gb_WinResTarget__command_target = $(WORKDIR)/LinkTarget/Executable/makedepend.exe
define gb_WinResTarget__command_dep
$(call gb_Output_announce,RC:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(call gb_Executable_get_target,makedepend) \
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

# Set INCLUDE and LIB variables and unset MAKE/MAKEFLAGS when using nmake
#
# gb_ExternalProject_use_nmake project state_target
define gb_ExternalProject_use_nmake
$(call gb_ExternalProject_get_state_target,$(1),$(2)): NMAKE := $(gb_NMAKE_VARS)
endef

# if ccache is enabled, then split it and use lastword as REAL_FOO
# /opt/lo/bin/ccache /cygdrive/c/PROGRA~2/MICROS~2.0/VC/bin/cl.exe

gb_AUTOCONF_WRAPPERS = \
	REAL_CC="$(shell cygpath -w $(filter-out -%,$(CC)))" \
	REAL_CC_FLAGS="$(filter -%,$(CC))" \
	CC="$(call gb_Executable_get_target,gcc-wrapper)" \
	REAL_CXX="$(shell cygpath -w $(filter-out -%,$(CXX)))" \
	REAL_CXX_FLAGS="$(filter -%,$(CXX))" \
	CXX="$(call gb_Executable_get_target,g++-wrapper)" \
    LD="$(shell cygpath -w $(COMPATH)/bin/link.exe) -nologo"

gb_ExternalProject_INCLUDE := \
	$(subst -I,,$(subst $(WHITESPACE),;,$(SOLARINC)))

gb_NMAKE_VARS = \
	CC="$(shell cygpath -w $(filter-out -%,$(CC))) $(filter -%,$(CC))" \
	INCLUDE="$(gb_ExternalProject_INCLUDE)" \
	LIB="$(ILIB)" \
	MAKEFLAGS= \
	MAKE=

# InstallScript class

gb_InstallScript_EXT := .inf

# CliAssemblyTarget class

gb_CliAssemblyTarget_POLICYEXT := $(gb_Library_DLLEXT)
gb_CliAssemblyTarget_get_dll = $(call gb_Library__get_dir_for_layer,NONE)/$(1)$(gb_CliAssemblyTarget_POLICYEXT)

# Extension class

gb_Extension_LICENSEFILE_DEFAULT := $(INSTROOT)/license.txt

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(shell cygpath -u $(TARFILE_LOCATION))

# UnoApiHeadersTarget class

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

# UIConfig class

# use responsefile because cui has too many files for command line
define gb_UIConfig__command
$(call gb_Helper_abbreviate_dirs,\
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,$(if $(UI_IMAGELISTS),$(strip $(UI_IMAGELISTS)),/dev/null)) \
	&& tr " " "\000" < $$RESPONSEFILE | tr -d "\r\n" > $$RESPONSEFILE.0 \
	&& $(SORT) -u --files0-from=$$RESPONSEFILE.0 > $@ \
	&& rm $$RESPONSEFILE $$RESPONSEFILE.0 \
)

endef

# use file list file because swriter has too many files for command line
define gb_UIConfig__gla11y_command
$(call gb_ExternalExecutale__check_registration,python)
$(call gb_Helper_abbreviate_dirs,\
	FILES=$(call var2file,$(shell $(gb_MKTEMP)),100,$(UIFILES)) && \
	$(gb_UIConfig_LXML_PATH) $(gb_Helper_set_ld_path) \
	$(call gb_ExternalExecutable_get_command,python) \
	$(gb_UIConfig_gla11y_SCRIPT) $(gb_UIConfig_gla11y_PARAMETERS) -o $@ -L $$FILES
)

endef

# UIMenubarTarget class

define gb_UIMenubarTarget__command
$(call gb_Output_announce,$(2),$(true),UIM,1)
cp $(3) $(1)

endef

gb_UIMenubarTarget_UIMenubarTarget_platform :=

# Python
gb_Python_PRECOMMAND := PATH="$(shell cygpath -w $(INSTDIR)/program)" PYTHONHOME="$(INSTDIR)/program/python-core-$(PYTHON_VERSION)" PYTHONPATH="$${PYPATH:+$$PYPATH:}$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib;$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib/lib-dynload:$(INSTDIR)/program"
gb_Python_INSTALLED_EXECUTABLE := $(INSTROOT)/$(LIBO_BIN_FOLDER)/python.exe

gb_ICU_PRECOMMAND := PATH="$(shell cygpath -w $(WORKDIR_FOR_BUILD)/UnpackedTarball/icu/source/lib)"

# vim: set noet sw=4:
