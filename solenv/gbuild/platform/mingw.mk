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

gb_RC := $(WINDRES)

gb_COMPILERDEFS += \
	-D_MT \
	-D_NATIVE_WCHAR_T_DEFINED \
	-D_MSC_EXTENSIONS \
	-D_FORCENAMELESSUNION \

gb_RCDEFS := \
	-DWINVER=0x0400 \
	-DWIN32 \

gb_RCFLAGS :=

gb_CFLAGS += \
	$(gb_CFLAGS_COMMON) \
	-Wdeclaration-after-statement \
	-fno-strict-aliasing \

# For -Wno-non-virtual-dtor see <http://markmail.org/message/664jsoqe6n6smy3b>
# "Re: [dev] warnings01: -Wnon-virtual-dtor" message to dev@openoffice.org from
# Feb 1, 2006:
gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-Wreturn-type \
	-Wshadow \
	-Wuninitialized \
	-fno-strict-aliasing \


ifneq ($(SYSBASE),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
endif

ifeq ($(HAVE_CXX11),TRUE)
gb_CXXFLAGS += -std=gnu++0x
endif

# At least sal defines its own __main, which would cause DLLs linking against
# sal to pick up sal's __main instead of the one from MinGW's dllcrt2.o:
gb_LinkTarget_LDFLAGS := \
	-Wl,--export-all-symbols \
	-Wl,--exclude-symbols,__main \
	-Wl,--enable-stdcall-fixup \
	-Wl,--enable-runtime-pseudo-reloc-v2 \
	$(SOLARLIB) \

ifeq ($(MINGW_GCCLIB_EH),YES)
gb_LinkTarget_LDFLAGS += -shared-libgcc
endif

gb_STDLIBS := \
	-lmingwthrd \
	-lmingw32 \
	-lmingwex \

gb_DEBUG_CFLAGS := -g -fno-inline

# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)

ifeq ($(gb_SYMBOL),$(true))
gb_LinkTarget_CXXFLAGS += $(GGDB2)
gb_LinkTarget_CFLAGS += $(GGDB2)
endif

gb_LinkTarget_INCLUDE +=\
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

define gb_LinkTarget__command_dynamiclinkexecutable
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(if $(CXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS),$(gb_CXX),$(gb_CC)) $(strip \
		$(gb_Executable_TARGETTYPEFLAGS) \
		$(if $(filter YES,$(TARGETGUI)), -mwindows, -mconsole) \
		$(T_LDFLAGS) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(NATIVERES) \
		$(if $(LINKED_STATIC_LIBS),-Wl$(COMMA)--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) -Wl$(COMMA)--end-group) \
		$(patsubst lib%.a,-l%,$(patsubst lib%.dll.a,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_ilibfilename,$(lib))))) \
		$(LIBS) \
		-Wl$(COMMA)-Map$(COMMA)$(WORKDIR)/LinkTarget/$(2).map \
		-o $(1)))
endef

define gb_LinkTarget__command_dynamiclinklibrary
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(if $(CXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS),$(gb_CXX),$(gb_CC)) $(strip \
		$(gb_Library_TARGETTYPEFLAGS) \
		$(T_LDFLAGS) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(NATIVERES) \
		$(if $(LINKED_STATIC_LIBS),-Wl$(COMMA)--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) -Wl$(COMMA)--end-group) \
		$(patsubst lib%.a,-l%,$(patsubst lib%.dll.a,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_ilibfilename,$(lib))))) \
		$(LIBS) \
		-Wl$(COMMA)-Map$(COMMA)$(WORKDIR)/LinkTarget/$(2).map \
		-Wl$(COMMA)--out-implib$(COMMA)$(ILIBTARGET) \
		-o $(1) \
		$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))
endef

define gb_LinkTarget__command_staticlinklibrary
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinkexecutable,$(1),$(2)))
$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinklibrary,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlinklibrary,$(1)))
endef

define gb_LinkTarget_use_system_win32_libs
$(call gb_LinkTarget_add_libs,$(1),$(foreach lib,$(2),-l$(patsubst oldnames,moldname,$(lib))))
endef

gb_LinkTarget_get_mapfile = \
$(WORKDIR)/LinkTarget/$(patsubst %.dll,%.map,$(call gb_LinkTarget__get_workdir_linktargetname,$(1)))

# Library class

gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := -shared
gb_Library_get_rpath :=

gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_UDK_MAJORVER := 3

gb_Library_PLAINEXT := .dll.a

gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)
gb_Library_RTVEREXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_RTEXT)
gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
gb_Library_UNOVEREXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_PLAINEXT)

gb_Library_DLLEXT := .dll

gb_Library_RTDLLEXT := gcc3$(gb_Library_DLLEXT)
gb_Library_RTVERDLLEXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_RTDLLEXT)
gb_Library_OOODLLEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_DLLEXT)
gb_Library_UNOVERDLLEXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_DLLEXT)

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_SHLXTHDL),$(lib):SHLXTHDL) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

gb_Library_ILIBFILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_SHLXTHDL),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOODLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_SHLXTHDL),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(lib)$(gb_Library_OOODLLEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVERDLLEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVERDLLEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \

gb_Library_IARCSYSPRE := lib
gb_Library_IARCEXT := .a

gb_Library_ILIBEXT := .lib

define gb_Library_Library_platform
$(call gb_LinkTarget_set_ilibtarget,$(2),$(3))

$(call gb_Library_get_target,$(1)) :| $(OUTDIR)/bin/.dir

$(call gb_LinkTarget_add_auxtargets,$(2), \
	$(3) \
	$(call gb_LinkTarget_get_mapfile,$(2)) \
)

$(call gb_Library_add_default_nativeres,$(1),$(1)/default)

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(2))
$(call gb_WinResTarget_set_rcfile,$(2),include/default)
$(call gb_WinResTarget_add_defs,$(2),\
		-DVERVARIANT="$(LIBO_VERSION_PATCH)" \
		-DRES_APP_VENDOR="$(OOO_VENDOR)" \
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

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETGUI :=
endef

# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := $(gb_Helper_set_ld_path)
gb_CppunitTest_get_filename = test_$(1).dll
gb_CppunitTest_get_ilibfilename = itest_$(1).lib

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_ilibtarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(3) \
	$(call gb_LinkTarget_get_mapfile,$(2)) \
)

endef

# WinResTarget class

gb_WinResTarget_POSTFIX :=.o

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RC ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		$(INCLUDE) \
		-o $(1) \
		$(RCFILE) )
endef

gb_WinResTarget_WinResTarget_platform :=

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
gb_WinResTarget__command_target = $(gb_Executable_BINDIR_FOR_BUILD)/makedepend
define gb_WinResTarget__command_dep
$(call gb_Output_announce,RC:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_Executable_BINDIR_FOR_BUILD)/makedepend \
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
	-DCOMID=gcc3 \
	-D_gcc3 \
)

endef

# InstallScript class

gb_InstallScript_EXT := .inf

# CliAssemblyTarget class

gb_CliAssemblyTarget_POLICYEXT :=
gb_CliAssemblyTarget_get_dll :=

# Extension class

gb_Extension_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/license.txt

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

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
gb_PYTHON_PRECOMMAND := PATH="$${PATH}:$(OUTDIR_FOR_BUILD)/bin" PYTHONHOME="$(OUTDIR_FOR_BUILD)/lib/python" PYTHONPATH="$(OUTDIR_FOR_BUILD)/lib/python;$(OUTDIR_FOR_BUILD)/lib/python/lib-dynload"

# vim: set noet sw=4:
