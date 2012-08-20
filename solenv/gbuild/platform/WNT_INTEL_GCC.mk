# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

gb_COMPILERDEFAULTOPTFLAGS := -Os
gb_CPUDEFS := -D_M_IX86
gb_TMPDIR:=$(if $(TMPDIR),$(TMPDIR),/tmp)

# please make generic modifications to either of these
include $(GBUILDDIR)/platform/com_GCC_defs.mk
include $(GBUILDDIR)/platform/windows.mk

gb_CCVER := $(shell $(gb_CC) -dumpversion | $(gb_AWK) -F. -- \
    '{ print $$1*10000+$$2*100+$$3 }')
gb_GccLess470 := $(shell expr $(gb_CCVER) \< 40700)

gb_RC := $(WINDRES)

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS += \
	-D_MT \
	-D_NATIVE_WCHAR_T_DEFINED \
	-D_MSC_EXTENSIONS \
	-D_FORCENAMELESSUNION \

# Until GCC 4.6, MinGW used __cdecl by default, and BOOST_MEM_FN_ENABLE_CDECL
# would result in ambiguous calls to overloaded boost::bind; since GCC 4.7,
# MinGW uses __thiscall by default, so now needs BOOST_MEM_FN_ENABLE_CDECL for
# uses of boost::bind with functions annotated with SAL_CALL:
ifeq ($(gb_GccLess470),0)
gb_COMPILERDEFS += -DBOOST_MEM_FN_ENABLE_CDECL
endif

gb_RCDEFS := \
	 -DWINVER=0x0400 \
	 -DWIN32 \

gb_RCFLAGS := \
	 -V

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

ifeq ($(HAVE_CXX0X),TRUE)
# We can turn on additional useful checks with c++0x
# FIXME still does not compile fully gb_CXXFLAGS += -std=gnu++0x
endif

# At least sal defines its own __main, which would cause DLLs linking against
# sal to pick up sal's __main instead of the one from MinGW's dllcrt2.o:
gb_LinkTarget_LDFLAGS := \
	-Wl,--export-all-symbols \
	-Wl,--exclude-symbols,__main \
	-Wl,--enable-stdcall-fixup \
	-Wl,--enable-runtime-pseudo-reloc-v2 \
	$(subst -L../lib , ,$(SOLARLIB)) \

ifeq ($(MINGW_GCCLIB_EH),YES)
gb_LinkTarget_LDFLAGS += -shared-libgcc
endif

gb_STDLIBS := \
	mingwthrd \
	mingw32 \
	uwinapi \
	mingwex \


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
	$(gb_CXX) $(strip \
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
		$(patsubst lib%.a,-l%,$(patsubst lib%.dll.a,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_implibname,$(lib))))) \
		$(LIBS) \
		-Wl$(COMMA)-Map$(COMMA)$(basename $(1)).map \
		-o $(1)))
endef

define gb_LinkTarget__command_dynamiclinklibrary
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_CXX) $(strip \
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
		$(patsubst lib%.a,-l%,$(patsubst lib%.dll.a,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_implibname,$(lib))))) \
		$(LIBS) \
		-Wl$(COMMA)-Map$(COMMA)$(basename $(DLLTARGET)).map \
		-Wl$(COMMA)--out-implib$(COMMA)$(1) \
		-o $(dir $(1))/$(notdir $(DLLTARGET))))
endef

define gb_LinkTarget__command_staticlinklibrary
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinkexecutable,$(1),$(2)))
$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinklibrary,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlinklibrary,$(1)))
endef


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
gb_Library_UNODLLEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVERDLLEXT := $(gb_Library_UDK_MAJORVER)$(gb_Library_DLLEXT)

gb_Library_win32_OLDNAMES := moldname

gb_Library_PLAINLIBS_NONE += \
	mingwthrd \
	mingw32 \
	mingwex \
	advapi32 \
	comctl32 \
	comdlg32 \
	crypt32 \
	d3d9 \
	d3dx \
	ddraw \
	gdi32 \
	gdiplus \
	gnu_getopt \
	imm32 \
	kernel32 \
	jawt \
	msi \
	msimg32 \
	msvcrt \
	msvcprt \
	mpr \
	$(gb_Library_win32_OLDNAMES) \
	ole32 \
	oleaut32 \
	propsys \
	shell32 \
	shlwapi \
	urlmon \
	user32 \
	usp10 \
	uuid \
	winmm \
	version \
	wininet \
	winspool \
	wldap32 \
	ws2_32 \

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_DLLFILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOODLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTDLLEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVERDLLEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNODLLEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNODLLEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVERDLLEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \

gb_Library_IARCSYSPRE := lib
gb_Library_IARCEXT := .a

gb_Library_ILIBEXT := .lib

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(OUTDIR)/bin/$(notdir $(3)))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.dll,%.map,$(3)) \
)

$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS := $(OUTDIR)/bin/$(notdir $(3))

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3),$(1))

$(call gb_Library_add_default_nativeres,$(1),$(1)/default)

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(2))
$(call gb_WinResTarget_set_rcfile,$(2),solenv/inc/shlinfo)
$(call gb_WinResTarget_add_defs,$(2),\
		-DVERVARIANT="$(BUILD)" \
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

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef

define gb_Library_get_implibname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))

gb_StaticLibrary_StaticLibrary_platform =

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETGUI :=
endef

# CppunitTest class

ifeq ($(CROSS_COMPILING),YES)
gb_CppunitTest_CPPTESTPRECOMMAND := :
else
gb_CppunitTest_CPPTESTPRECOMMAND := $(gb_Helper_set_ld_path)
endif
gb_CppunitTest_SYSPRE := itest_
gb_CppunitTest_EXT := .lib
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = test_$(1).dll

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_add_auxtargets,$(2),\
	$(patsubst %.dll,%.map,$(3)) \
)

endef

# WinResTarget class

gb_WinResTarget_POSTFIX :=.o

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RES,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		$(INCLUDE) \
		-o $(1) \
		$(RCFILE) )
endef

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
define gb_WinResTarget__command_dep
$(call gb_Output_announce,RES:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(OUTDIR_FOR_BUILD)/bin/makedepend \
		$(INCLUDE) \
		$(DEFS) \
		$(RCFILE) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(3) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
	> $(1))
endef
else
gb_WinResTarget__command_dep =
endif

# InstallModuleTarget class

define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=gcc3 \
	-DCOMNAME=gcc3 \
	-D_gcc3 \
	$(if $(filter TRUE,$(SOLAR_JAVA)),-DSOLAR_JAVA) \
)

$(call gb_InstallModuleTarget_set_include,$(1),\
	$(SOLARINC) \
	$(SCP_INCLUDE) \
)

endef

# ScpConvertTarget class

define gb_ScpConvertTarget_ScpConvertTarget_platform
$(call gb_ScpConvertTarget_get_target,$(1)) :| $(OUTDIR)/bin/msi-encodinglist.txt
$(call gb_ScpConvertTarget_get_target,$(1)) : SCP_FLAGS := -t $(OUTDIR)/bin/msi-encodinglist.txt

endef

# InstallScript class

gb_InstallScript_EXT := .inf

# ExtensionTarget class

gb_ExtensionTarget_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/license.txt

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

# Python
gb_PYTHON_PRECOMMAND := PATH="$${PATH}:$(OUTDIR_FOR_BUILD)/bin" PYTHONHOME="$(OUTDIR_FOR_BUILD)/lib/python" PYTHONPATH="$(OUTDIR_FOR_BUILD)/lib/python;$(OUTDIR_FOR_BUILD)/lib/python/lib-dynload"

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
