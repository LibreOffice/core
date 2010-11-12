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

GUI := UNX
COM := GCC

gb_MKTEMP := mktemp -p

gb_CC := gcc
gb_CXX := g++
gb_GCCP := gcc

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

gb_OSDEFS := \
    -DLINUX \
    -D_PTHREADS \
    -DUNIX \
    -DUNX \
    $(PTHREAD_CFLAGS) \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS := \
    -DGCC \
    -D$(CVER) \
    -DCVER=$(CVER) \
    -DGLIBC=2 \
    -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \
    -DHAVE_GCC_VISIBILITY_FEATURE \
    -DCPPU_ENV=gcc3 \

ifeq ($(CPUNAME),X86_64)
gb_CPUDEFS := -D$(CPUNAME)
else
gb_CPUDEFS := -DX86
endif

gb_CFLAGS := \
    -Wall \
    -Wendif-labels \
    -Wextra \
    -fmessage-length=0 \
    -fno-strict-aliasing \
    -fpic \
    -fvisibility=hidden \
    -pipe \

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
    -fpic \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden \
    -pipe \

ifneq ($(SYSBASE),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
endif
gb_LinkTarget_EXCEPTIONFLAGS := \
    -DEXCEPTIONS_ON \
    -fexceptions \
    -fno-enforce-eh-specs \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
    -DEXCEPTIONS_OFF \
    -fno-exceptions \
    
gb_LinkTarget_LDFLAGS := \
    -Wl,--sysroot=$(SYSBASE) \
    -Wl,-rpath-link=$(SOLARLIBDIR):$(SYSBASE)/lib:$(SYSBASE)/usr/lib \
    -Wl,--hash-style=both \
    -Wl,-z,combreloc \
    -Wl,-z,defs \
    $(subst -L../lib , ,$(SOLARLIB)) \

ifneq ($(HAVE_LD_BSYMBOLIC_FUNCTIONS),)
gb_LinkTarget_LDFLAGS += \
    -Wl,--dynamic-list-cpp-new \
    -Wl,--dynamic-list-cpp-typeinfo \
    -Wl,-Bsymbolic-functions \

endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := -Os
endif


# Helper class

gb_Helper_abbreviate_dirs_native = $(gb_Helper_abbreviate_dirs)
gb_Helper_GNUCOPY := cp


# CObject class

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    mkdir -p $(dir $(call gb_CObject_get_dep_target,$(2))) && \
    $(gb_CC) \
        -c $(3) \
        -o $(1) \
        -MMD -MT $(call gb_CObject_get_target,$(2)) \
        -MF $(call gb_CObject_get_dep_target,$(2)) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6))
endef


# CxxObject class

define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    mkdir -p $(dir $(call gb_CxxObject_get_dep_target,$(2))) && \
    $(gb_CXX) \
        -c $(3) \
        -o $(1) \
        -MMD -MT $(call gb_CxxObject_get_target,$(2)) \
        -MF $(call gb_CxxObject_get_dep_target,$(2)) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6))
endef


# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef
define gb_LinkTarget_get_rpath
-Wl,-rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_LinkTarget_get_layer,$(2))) \
-Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)
endef

gb_LinkTarget__RPATHS := \
    URELIB:'$$$$ORIGIN' \
    UREBIN:'$$$$ORIGIN/../lib:$$$$ORIGIN' \
    OOOLIB:'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' \
    BRAND:'$$$$ORIGIN:$$$$ORIGIN/../basis-link/program:$$$$ORIGIN/../basis-link/ure-link/lib' \
    SDKBIN:'$$$$ORIGIN/../../ure-link/lib' \
    NONEBIN:'$$$$ORIGIN/../lib:$$$$ORIGIN' \

gb_LinkTarget_LAYER := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_STLLIBS),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
    $(foreach lib,$(gb_Executable_UREBIN),$(lib):UREBIN) \
    $(foreach lib,$(gb_Executable_SDK),$(lib):SDKBIN) \
    $(foreach lib,$(gb_Executable_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Executable_BRAND),$(lib):BRAND) \
    $(foreach lib,$(gb_Executable_NONE),$(lib):NONEBIN) \

gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_COMPILEROPTFLAGS)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CXXFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline
gb_LinkTarget_CFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline

endif

gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    $(gb_CXX) \
        $(3) \
        $(patsubst lib%.so,-l%,$(foreach lib,$(4),$(call gb_Library_get_filename,$(lib)))) \
        $(foreach object,$(6),$(call gb_CObject_get_target,$(object))) \
        $(foreach object,$(7),$(call gb_CxxObject_get_target,$(object))) \
        -Wl$(COMMA)--start-group $(foreach lib,$(5),$(call gb_StaticLibrary_get_target,$(lib))) -Wl$(COMMA)--end-group \
        -o $(1))
endef


# Library class

gb_Library_DEFS := -DSHAREDLIB -D_DLL_
gb_Library_TARGETTYPEFLAGS := -shared -Wl,-z,noexecstack
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .so
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_gcc$(gb_Library_PLAINEXT)
else
gb_Library_STLEXT := port_gcc_stldebug$(gb_Library_PLAINEXT)
endif

ifeq ($(CPUNAME),X86_64)
gb_Library_OOOEXT := lx$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
else
gb_Library_OOOEXT := li$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
endif

gb_Library_PLAINLIBS += \
    dl \
    jpeg \
    m \
    pthread \
    X11 \
    z

gb_Library_FILENAMES := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_STLLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_STLEXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \


gb_Library_Library_platform =


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS := -Wl,-z,noexecstack -static -nostdlib
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
    $(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
    $(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \


# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable_Executable_platform =


# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib


# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc
gb_SrsPartTarget_RSCCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
    $(gb_GCCP) \
        -MM -MT $(call gb_SrsPartTarget_get_target,$(1)) \
        $(3) \
        $(4) \
        -c -x c++-header $(2) \
        -o $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef


# ComponentTarget

gb_ComponentTarget_XSLTPROCPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_ComponentTarget_PREFIXBASISNATIVE := vnd.sun.star.expand:$$OOO_BASE_DIR/program/


# vim: set noet sw=4 ts=4:
