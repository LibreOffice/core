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
COM := MSC

gb_CC := cl
gb_CXX := cl
gb_LINK := link

gb_OSDEFS :=\
    -DWINVER=0x0500 \
    -D_WIN32_IE=0x0500 \
    -DNT351 \
    -DWIN32 \
    -DWNT \

gb_COMPILERDEFS :=\
    -DMSC \
    -D_CRT_NON_CONFORMING_SWPRINTFS \
    -D_CRT_NONSTDC_NO_DEPRECATE \
    -D_CRT_SECURE_NO_DEPRECATE \
    -D_MT \
    -DBOOST_MEM_FN_ENABLE_CDECL \
    -DCPPU_ENV=msci \
    -DFULL_DESK \
    -DM1500 \

gb_CPUDEFS := -DINTEL -D_X86_=1

gb_CFLAGS :=\
    -Gd \
    -GR \
    -Gs \
    -Gy \
    -nologo \
    -Wall \
    -wd4005 \
    -wd4061 \
    -wd4127 \
    -wd4180 \
    -wd4189 \
    -wd4191 \
    -wd4217 \
    -wd4250 \
    -wd4251 \
    -wd4255 \
    -wd4275 \
    -wd4290 \
    -wd4294 \
    -wd4350 \
    -wd4355 \
    -wd4365 \
    -wd4503 \
    -wd4505 \
    -wd4511 \
    -wd4512 \
    -wd4514 \
    -wd4611 \
    -wd4619 \
    -wd4625 \
    -wd4626 \
    -wd4640 \
    -wd4668 \
    -wd4675 \
    -wd4692 \
    -wd4710 \
    -wd4711 \
    -wd4738 \
    -wd4786 \
    -wd4800 \
    -wd4820 \
    -wd4826 \
    -WX \
    -Zc:forScope,wchar_t- \
    -Zm500 \

gb_CXXFLAGS :=\
    -Gd \
    -GR \
    -Gs \
    -Gy \
    -nologo \
    -Wall \
    -wd4005 \
    -wd4061 \
    -wd4127 \
    -wd4180 \
    -wd4189 \
    -wd4191 \
    -wd4217 \
    -wd4250 \
    -wd4251 \
    -wd4275 \
    -wd4290 \
    -wd4294 \
    -wd4350 \
    -wd4355 \
    -wd4365 \
    -wd4503 \
    -wd4505 \
    -wd4511 \
    -wd4512 \
    -wd4514 \
    -wd4611 \
    -wd4619 \
    -wd4625 \
    -wd4626 \
    -wd4640 \
    -wd4668 \
    -wd4675 \
    -wd4692 \
    -wd4710 \
    -wd4711 \
    -wd4738 \
    -wd4786 \
    -wd4800 \
    -wd4820 \
    -wd4826 \
    -WX \
    -Zc:forScope,wchar_t- \
    -Zm500 \

gb_LinkTarget_EXCEPTIONFLAGS :=\
    -DEXCEPTIONS_ON \
    -EHa \

gb_LinkTarget_NOEXCEPTIONFLAGS :=\
    -DEXCEPTIONS_OFF \
    
gb_LinkTarget_LDFLAGS :=\
    -MACHINE:IX86 \
    -NODEFAULTLIB \
    -SUBSYSTEM:CONSOLE \


ifeq ($(gb_DEBUGLEVEL),2)
gb_MSCOPTFLAGS :=
gb_LinkTarget_LDFLAGS += -DEBUG
else
gb_MSCOPTFLAGS := -Ob1 -Oxs -Oy-
endif


# Helper class

gb_Helper_SRCDIR_NATIVE := $(shell cygpath -m $(SRCDIR))
gb_Helper_WORKDIR_NATIVE := $(shell cygpath -m $(WORKDIR))
gb_Helper_OUTDIR_NATIVE := $(shell cygpath -m $(OUTDIR))

define gb_Helper_abbreviate_dirs_native
S=$(gb_Helper_SRCDIR_NATIVE) && \
$(subst $(gb_Helper_SRCDIR_NATIVE)/,$$S/,W=$(gb_Helper_WORKDIR_NATIVE) && O=$(gb_Helper_OUTDIR_NATIVE)) && \
$(subst $(SRCDIR)/,$$S/,$(subst $(WORKDIR)/,$$W/,$(subst $(OUTDIR)/,$$O/,$(1))))
endef

# CObject class

define gb_CObject__command
$(call gb_Helper_announce,Compiling $(2) (plain C) ...)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    C="$(gb_CC) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6) \
        -c $(3) \
        -Fo$(1)" && E=$$($$C) || (echo $$C && echo $$E 1>&2 && false))
endef

define gb_CObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_CObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)
endef


# CxxObject class

define gb_CxxObject__command
$(call gb_Helper_announce,Compiling $(2) ...)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    C="$(gb_CXX) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6) \
        -c $(3) \
        -Fo$(1)" && E=$$($$C) || (echo $$C && echo $$E 1>&2 && false))
endef

define gb_CxxObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_CObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)
endef


# LinkTarget class

gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_MSCOPTFLAGS)
gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_MSCOPTFLAGS)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CXXFLAGS +=
gb_LinkTarget_CFLAGS +=

endif

gb_LinkTarget_INCLUDE :=\
    $(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
    $(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command
$(call gb_Helper_announce,Linking $(2) ...)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    RESPONSEFILE=$$(mktemp --tmpdir=$(gb_Helper_MISC)) && \
    echo "$(foreach object,$(6),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(7),$(call gb_CObject_get_target,$(object)))" > $${RESPONSEFILE} && \
    $(gb_LINK) \
        $(3) \
        @$${RESPONSEFILE} \
        $(foreach lib,$(4),$(call gb_Library_get_filename,$(lib))) \
        $(foreach lib,$(5),$(call gb_StaticLibrary_get_filename,$(lib))) \
        $(subst -out: -implib:$(1),-out:$(1),-out:$(DLLTARGET) -implib:$(1)) && \
    rm $${RESPONSEFILE})

endef


# Library class

gb_Library_DEFS := -DSHAREDLIBS -D_DLL_ -D_DLL
gb_Library_TARGETTYPEFLAGS := -DLL

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_PLAINLIBS += \
    advapi32 \
    gdi32 \
    kernel32 \
    msvcrt \
    mpr \
    oldnames \
    ole32 \
    oleaut32 \
    shell32 \
    unicows \
    user32 \
    uuid \
    uwinapi \
    z \

# HACK
# .lib files should be named following the same scheme that the .dll use in the end
gb_Library_FILENAMES :=\
    $(foreach lib,$(gb_Library_TARGETS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

# HACKS
gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cui:icui%,cui:icuin%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:iii18nisolang1%,i18nisolang1:iii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst stl:istl%,stl:stlport_vc71%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:ivos3%,vos3:ivos%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))
gb_Library_NOILIBFILENAMES:=\
    advapi32 \
    gdi32 \
    icuuc \
    kernel32 \
    msvcrt \
    mpr \
    oldnames \
    ole32 \
    oleaut32 \
    shell32 \
    sot \
    unicows \
    user32 \
    uuid \
    uwinapi \

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))

gb_Library_DLLEXT := .dll
gb_Library_MAJORVER := 3
gb_Library_RTEXT := MSC$(gb_Library_DLLEXT)
gb_Library_STLEXT := port_vc7145$(gb_Library_DLLEXT)
gb_Library_OOOEXT := mi$(gb_Library_DLLEXT)
gb_Library_UNOEXT := mi.uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES :=\
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS),$(lib):$(lib)$(gb_Library_DLLEXT)) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
    $(foreach lib,$(gb_Library_STLLIBS),$(lib):$(lib)$(gb_Library_STLEXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \

# HACKS
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp4%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbhelper4%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))


define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),\
    $(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
    $(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
    $(3).manifest \
)

endef

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS :=
gb_StaticLibrary_SYSPRE :=
gb_StaticLibrary_PLAINEXT := .lib
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
    $(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
    $(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))


# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS := -RELEASE -BASE:0x1b000000 -OPT:NOREF -INCREMENTAL:NO -DEBUG

define gb_Executable_Executable_platform
$(call gb_LinkTarget_set_auxtargets,$(2),\
    $(patsubst %.exe,%.pdb,$(call gb_LinkTarget_get_target,$(2))) \
    $(call gb_LinkTarget_get_target,$(2)).manifest \
)
endef

# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"


# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc.exe
gb_SrsPartTarget_RSCCOMMAND := SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

define gb_SrsPartTarget__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_SrsPartTarget_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)
endef


# vim: set noet sw=4 ts=4:
