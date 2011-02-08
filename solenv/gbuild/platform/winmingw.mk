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

GUI := WNT
COM := GCC

gb_MKTEMP := mktemp -p

gb_CC := $(CC)
gb_CXX := $(CXX)
gb_LINK := $(shell $(CC) -print-prog-name=ld)
gb_AR := $(shell $(CC) -print-prog-name=ar)
gb_AWK := awk
ifeq ($(USE_MINGW),cygwin)
gb_MINGWLIBDIR := $(COMPATH)/lib/mingw
else
ifeq ($(USE_MINGW),cygwin-w64-mingw32)
gb_MINGWLIBDIR := $(COMPATH)/usr/i686-w64-mingw32/sys-root/mingw/lib
else
gb_MINGWLIBDIR := $(COMPATH)/lib
endif
endif
ifeq ($(MINGW_SHARED_GXXLIB),YES)
gb_MINGW_LIBSTDCPP := $(subst -l,,$(MINGW_SHARED_LIBSTDCPP))
else
gb_MINGW_LIBSTDCPP := \
        stdc++ \
        moldname
endif
ifeq ($(MINGW_SHARED_GCCLIB),YES)
gb_MINGW_LIBGCC := \
        gcc_s \
        gcc
else
ifeq ($(MINGW_GCCLIB_EH),YES)
gb_MINGW_LIBGCC := \
        gcc \
        gcc_eh
else
gb_MINGW_LIBGCC := gcc
endif
endif

gb_OSDEFS := \
    -DWINVER=0x0500 \
    -D_WIN32_IE=0x0500 \
    -DNT351 \
    -DWIN32 \
    -DWNT \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS := \
    -DGCC \
    -D$(CVER) \
    -DCVER=$(CVER) \
    -DGLIBC=2 \
    -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \
    -DCPPU_ENV=gcc3 \
    -D_MT \
    -D_NATIVE_WCHAR_T_DEFINED \
    -D_MSC_EXTENSIONS \
    -D_FORCENAMELESSUNION \

ifeq ($(USE_MINGW),cygwin-w64-mingw32)
gb_COMPILERDEFS +=-D_declspec=__declspec
endif

gb_CPUDEFS := \
    -DINTEL \
    -D_M_IX86 \

gb_CFLAGS := \
    -Wall \
    -Wendif-labels \
    -Wextra \
    -fmessage-length=0 \
    -fno-strict-aliasing \
    -pipe \
    -nostdinc \

gb_CXXFLAGS := \
    -Wall \
    -Wendif-labels \
    -Wextra \
    -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor \
    -Wreturn-type \
    -Wshadow \
    -Wuninitialized \
    -fmessage-length=0 \
    -fno-strict-aliasing \
    -fno-use-cxa-atexit \
    -pipe \
    -nostdinc \

ifneq ($(SYSBASE),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
endif
gb_LinkTarget_EXCEPTIONFLAGS := \
    -DEXCEPTIONS_ON \
    -fexceptions \
    -fno-enforce-eh-specs \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)


gb_LinkTarget_NOEXCEPTIONFLAGS := \
    -DEXCEPTIONS_OFF \
    -fno-exceptions \
    
gb_NoexPrecompiledHeader_NOEXCEPTIONFLAGS := $(gb_LinkTarget_NOEXCEPTIONFLAGS)

gb_LinkTarget_LDFLAGS := \
    --export-all-symbols \
    --kill-at \
    --subsystem console \
    --exclude-libs ALL \
    --enable-stdcall-fixup \
    --enable-runtime-pseudo-reloc-v2 \
    -L$(gb_Library_DLLDIR) \
    $(patsubst %,-L%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

ifeq ($(MINGW_GCCLIB_EH),YES)
gb_LinkTarget_LDFLAGS += -shared-libgcc
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := -Os
endif


# Helper class

gb_Helper_SRCDIR_NATIVE := $(shell cygpath -m $(SRCDIR))
gb_Helper_WORKDIR_NATIVE := $(shell cygpath -m $(WORKDIR))
gb_Helper_OUTDIR_NATIVE := $(shell cygpath -m $(OUTDIR))
gb_Helper_REPODIR_NATIVE := $(shell cygpath -m $(REPODIR))

define gb_Helper_abbreviate_dirs_native
R=$(gb_Helper_REPODIR_NATIVE) && \
$(subst $(gb_Helper_REPODIR_NATIVE)/,$$R/,S=$(gb_Helper_SRCDIR_NATIVE) && \
$(subst $(gb_Helper_SRCDIR_NATIVE)/,$$S/,O=$(gb_Helper_OUTDIR_NATIVE)) && \
$(subst $(gb_Helper_SRCDIR_NATIVE)/,$$S/,$(subst $(REPODIR)/,$$R/,$(subst $(SRCDIR)/,$$S/,$(subst $(gb_Helper_OUTDIR_NATIVE)/,$$O/,$(subst $(OUTDIR)/,$$O/,W=$(gb_Helper_WORKDIR_NATIVE) && $(subst $(gb_Helper_WORKDIR_NATIVE)/,$$W/,$(subst $(WORKDIR)/,$$W/,$(1)))))))))
endef

# CObject class

ifeq ($(gb_FULLDEPS),$(true))
define gb_CObject__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
    $(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
        $(filter-out -DPRECOMPILED_HEADERS,$(4)) $(5) \
        -I$(dir $(3)) \
        $(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME),$(6)) \
        $(3) \
        -f - \
    | $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
        -v OBJECTFILE=$(1) \
        -v OUTDIR=$(OUTDIR)/ \
        -v WORKDIR=$(WORKDIR)/ \
        -v SRCDIR=$(SRCDIR)/ \
        -v REPODIR=$(REPODIR)/ \
    > $(call gb_CObject_get_dep_target,$(2)))
endef
else
CObject__command_deponcompile =
endif

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    unset INCLUDE && \
    $(gb_CC) \
        -c $(3) \
        -o $(1) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6))
$(call gb_CObject__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))
endef



# CxxObject class

ifeq ($(gb_FULLDEPS),$(true))
define gb_CxxObject__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
    $(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
        $(filter-out -DPRECOMPILED_HEADERS,$(4)) $(5) \
        -I$(dir $(3)) \
        $(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME),$(6)) \
        $(3) \
        -f - \
    | $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
        -v OBJECTFILE=$(1) \
        -v OUTDIR=$(OUTDIR)/ \
        -v WORKDIR=$(WORKDIR)/ \
        -v SRCDIR=$(SRCDIR)/ \
        -v REPODIR=$(REPODIR)/ \
    > $(call gb_CxxObject_get_dep_target,$(2)))
endef
else
gb_CxxObject__command_deponcompile =
endif

define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    unset INCLUDE && \
    $(gb_CXX) \
        -c $(3) \
        -o $(1) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6))
$(call gb_CxxObject__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))

endef


# PrecompiledHeader class

gb_PrecompiledHeader_EXT := .gch

gb_PrecompiledHeader_get_enableflags = -I$(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR) \
                    -DPRECOMPILED_HEADERS \
                    -Winvalid-pch \

ifeq ($(gb_FULLDEPS),$(true))
define gb_PrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
    $(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(filter-out -I$(COMPATH)% -I$(JAVA_HOME),$(6)) \
        $(3) \
        -f - \
    | $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
        -v OBJECTFILE=$(1) \
        -v OUTDIR=$(OUTDIR)/ \
        -v WORKDIR=$(WORKDIR)/ \
        -v SRCDIR=$(SRCDIR)/ \
        -v REPODIR=$(REPODIR)/ \
    > $(call gb_PrecompiledHeader_get_dep_target,$(2)))
endef
else
gb_PrecompiledHeader__command_deponcompile =
endif

define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2))) && \
    unset INCLUDE && \
    $(gb_CXX) \
        -x c++-header \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6) \
        -c $(3) \
        -o$(1))
$(call gb_PrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))

endef

# NoexPrecompiledHeader class

gb_NoexPrecompiledHeader_EXT := .gch

gb_NoexPrecompiledHeader_get_enableflags = -I$(WORKDIR)/NoexPrecompiledHeader/$(gb_NoexPrecompiledHeader_DEBUGDIR) \
                    -Winvalid-pch \

ifeq ($(gb_FULLDEPS),$(true))
define gb_NoexPrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
    $(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(filter-out -I$(COMPATH)% -I$(JAVA_HOME),$(6)) \
        $(3) \
        -f - \
    | $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
        -v OBJECTFILE=$(1) \
        -v OUTDIR=$(OUTDIR)/ \
        -v WORKDIR=$(WORKDIR)/ \
        -v SRCDIR=$(SRCDIR)/ \
        -v REPODIR=$(REPODIR)/ \
    > $(call gb_NoexPrecompiledHeader_get_dep_target,$(2)))
endef
else
gb_NoexPrecompiledHeader__command_deponcompile =
endif

define gb_NoexPrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) $(dir $(call gb_NoexPrecompiledHeader_get_dep_target,$(2))) && \
    unset INCLUDE && \
    $(gb_CXX) \
        -x c++-header \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6) \
        -c $(3) \
        -o$(1))
$(call gb_NoexPrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))

endef


# LinkTarget class

gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_COMPILEROPTFLAGS)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CXXFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline
gb_LinkTarget_CFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline

endif

gb_LinkTarget_INCLUDE :=\
    $(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
    $(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command
$(if $(subst -static $(LDFLAGS),,$(TARGETTYPE_FLAGS) $(LDFLAGS)),$(if $(DLLTARGET),$(call gb_SharedLinkTarget__command,$(1),$(2),$(3),$(4),$(5),$(6),$(7)),$(call gb_ExecutableLinkTarget__command,$(1),$(2),$(3),$(4),$(5),$(6),$(7))),$(call gb_StaticLinkTarget__command,$(1),$(2),$(6),$(7)))
endef

define gb_SharedLinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    RESPONSEFILE=$$(mktemp --tmpdir=$(gb_Helper_MISC)) && \
    echo "$(foreach object,$(7),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(6),$(call gb_CObject_get_target,$(object)))" > $${RESPONSEFILE} && \
    $(gb_LINK) \
        $(3) \
        --enable-auto-image-base \
        -e _DllMainCRTStartup@12 \
        $(gb_MINGWLIBDIR)/dllcrt2.o \
        $(MINGW_CLIB_DIR)/crtbegin.o \
        @$${RESPONSEFILE} \
        --start-group $(foreach lib,$(5),$(call gb_StaticLibrary_get_target,$(lib))) --end-group \
        --start-group $(patsubst %.dll,-l%,$(foreach lib,$(4),$(call gb_Library_get_dllname,$(lib)))) --end-group \
        $(MINGW_CLIB_DIR)/crtend.o \
        -Map $(basename $(DLLTARGET)).map \
        -o $(DLLTARGET) && touch $(1))
endef

define gb_ExecutableLinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    RESPONSEFILE=$$(mktemp --tmpdir=$(gb_Helper_MISC)) && \
    echo "$(foreach object,$(7),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(6),$(call gb_CObject_get_target,$(object)))" > $${RESPONSEFILE} && \
    $(gb_LINK) \
        $(3) \
        $(gb_MINGWLIBDIR)/crt2.o \
        $(MINGW_CLIB_DIR)/crtbegin.o \
        @$${RESPONSEFILE} \
        --start-group $(foreach lib,$(5),$(call gb_StaticLibrary_get_target,$(lib))) --end-group \
        --start-group $(patsubst %.dll,-l%,$(foreach lib,$(4),$(call gb_Library_get_dllname,$(lib)))) --end-group \
        $(MINGW_CLIB_DIR)/crtend.o \
        -Map $(basename $(1)).map \
        -o $(1))
endef

define gb_StaticLinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    RESPONSEFILE=$$(mktemp --tmpdir=$(gb_Helper_MISC)) && \
    echo "$(foreach object,$(7),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(6),$(call gb_CObject_get_target,$(object)))" > $${RESPONSEFILE} && \
    $(gb_AR) -rsu\
        $(1) \
        @$${RESPONSEFILE})
endef


# Library class

gb_Library_DEFS := -D_DLL_ -D_DLL
gb_Library_TARGETTYPEFLAGS := -shared
gb_Library_get_rpath :=

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_PLAINLIBS_NONE += \
    mingwthrd \
    mingw32 \
    mingwex \
    $(gb_MINGW_LIBSTDCPP) \
    $(gb_MINGW_LIBGCC) \
    advapi32 \
    gdi32 \
    kernel32 \
    msvcrt \
    mpr \
    moldname \
    ole32 \
    oleaut32 \
    shell32 \
    unicows \
    user32 \
    uuid \
    uwinapi \
    z \

gb_LinkTarget_LAYER := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):OOOLIB) \

gb_Library_FILENAMES :=\
    $(foreach lib,$(gb_Library_TARGETS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

gb_Library_DLLEXT := .dll
gb_Library_MAJORVER := 3
gb_Library_RTEXT := gcc3$(gb_Library_DLLEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_gcc$(gb_Library_DLLEXT)
else
gb_Library_STLEXT := port_gcc_stldebug$(gb_Library_DLLEXT)
endif
gb_Library_OOOEXT := gi$(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \

gb_Library_IARCSYSPRE := lib
gb_Library_IARCEXT := .a

gb_Library_ILIBEXT := .lib

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
    $(patsubst %.dll,%.map,$(3)) \
)

$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS := $(OUTDIR)/bin/$(notdir $(3))

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3))

endef

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS := -static
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
    $(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
    $(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETTYPE := staticlib
$(call gb_LinkTarget_get_target,$(2)) : LAYER :=

endef

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable_get_rpath :=
gb_Executable_Executable_platform =


# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"

# SrsPartMergeTarget

gb_SrsPartMergeTarget_TRANSEXPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"

# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc.exe
gb_SrsPartTarget_RSCCOMMAND := SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

ifeq ($(gb_FULLDEPS),$(true))
define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs_native,\
    $(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
        $(3) $(4) \
        $(2) \
        -f - \
    | $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
        -v OBJECTFILE=$(call gb_SrsPartTarget_get_target,$(1)) \
        -v OUTDIR=$(OUTDIR)/ \
        -v WORKDIR=$(WORKDIR)/ \
        -v SRCDIR=$(SRCDIR)/ \
        -v REPODIR=$(REPODIR)/ \
    > $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef
else
gb_SrsPartTarget__command_dep =
endif

# ComponentTarget

gb_ComponentTarget_XSLTPROCPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"
gb_ComponentTarget_PREFIXBASISNATIVE := vnd.sun.star.expand:$$BRAND_BASE_DIR/program/

# vim: set noet sw=4 ts=4:
