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

gb_MKTEMP := TMPDIR= /usr/bin/mktemp -t

gb_CC := $(CC)
gb_CXX := $(CXX)
gb_GCCP := $(CC)
gb_AWK := awk


gb_OSDEFS := \
    -D$(OS) \
    -D$(GUI) \
    -DUNIX \
    -D_PTHREADS \
    -D_REENTRANT \
    -DNO_PTHREAD_PRIORITY \
    -DQUARTZ \
    $(EXTRA_CDEFS) \
#	$(PTHREAD_CFLAGS) \

gb_COMPILERDEFS := \
    -D$(COM) \
    -DGLIBC=2 \
    -D_USE_NAMESPACE=1 \
    -DHAVE_GCC_VISIBILITY_FEATURE \
    -DCPPU_ENV=gcc3 \
    -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \
#	-D$(CVER) \
#	-DCVER=$(CVER) \

ifeq ($(CPUNAME),INTEL)
gb_CPUDEFS := -DX86
else # ifeq ($(CPUNAME),POWERPC)
gb_CPUDEFS := -DPOWERPC -DPPC
endif

ifeq ($(strip $(SYSBASE)),)
gb_SDKDIR := /Developer/SDKs/MacOSX10.4u.sdk
else
gb_SDKDIR := $(SYSBASE)/MacOSX10.4u.sdk
endif


gb_CFLAGS := \
    -isysroot $(gb_SDKDIR) \
    -fsigned-char \
    -fmessage-length=0 \
    -malign-natural \
    -Wall \
    -Wendif-labels \
    -fno-strict-aliasing \
    -fPIC \
    -fno-common \
    -pipe \
#	-fvisibility=hidden \

gb_CXXFLAGS := \
    -isysroot $(gb_SDKDIR) \
    -fsigned-char \
    -fmessage-length=0 \
    -malign-natural \
    -Wall \
    -Wendif-labels \
    -Wno-long-double \
    -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor \
    -fno-strict-aliasing \
    -fPIC \
    -fno-common \
    -pipe \
#	-fvisibility=hidden \
#	-fvisibility-inlines-hidden \

# these are to get g++ to switch to Objective-C++ mode
# (see toolkit module for a case where it is necessary to do it this way)
gb_OBJCXXFLAGS := -x objective-c++ -fobjc-exceptions

gb_CFLAGS_WERROR := \
    -Werror \

gb_CXXFLAGS_WERROR := \
    -Werror \

gb_LinkTarget_EXCEPTIONFLAGS := \
    -DEXCEPTIONS_ON \
    -fexceptions \
    -fno-enforce-eh-specs \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
    -DEXCEPTIONS_OFF \
    -fno-exceptions \

gb_LinkTarget_LDFLAGS := \
    -Wl,-syslibroot,$(gb_SDKDIR) \
    $(subst -L../lib , ,$(SOLARLIB)) \
#man ld says: obsolete	-Wl,-multiply_defined,suppress \

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := -O2
endif


# Helper class

gb_Helper_abbreviate_dirs_native = $(gb_Helper_abbreviate_dirs)


# CObject class

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    mkdir -p $(dir $(call gb_CObject_get_dep_target,$(2))) && \
    $(gb_CC) \
        $(4) $(5) \
        -c $(3) \
        -o $(1) \
        -MMD -MT $(call gb_CObject_get_target,$(2)) \
        -MF $(call gb_CObject_get_dep_target,$(2)) \
        -I$(dir $(3)) \
        $(6))
endef


# CxxObject class

# N.B: $(4) or $(5) may contain -x objective-c++, which must come before -c
define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    mkdir -p $(dir $(call gb_CxxObject_get_dep_target,$(2))) && \
    $(gb_CXX) \
        $(4) $(5) \
        -c $(3) \
        -o $(1) \
        -MMD -MT $(call gb_CxxObject_get_target,$(2)) \
        -MF $(call gb_CxxObject_get_dep_target,$(2)) \
        -I$(dir $(3)) \
        $(6))
endef


# ObjCxxObject class

define gb_ObjCxxObject__command
$(call gb_Output_announce,$(2),$(true),OCX,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    mkdir -p $(dir $(call gb_ObjCxxObject_get_dep_target,$(2))) && \
    $(gb_CXX) \
        $(4) $(5) \
        -c $(3) \
        -o $(1) \
        -MMD -MT $(call gb_ObjCxxObject_get_target,$(2)) \
        -MF $(call gb_ObjCxxObject_get_dep_target,$(2)) \
        -I$(dir $(3)) \
        $(6))
endef


# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget_RPATHS)))
endef

gb_LinkTarget_RPATHS := \
    URELIB:@__________________________________________________URELIB/ \
    UREBIN: \
    OOOLIB:@__________________________________________________OOO/ \
    BRAND: \
    SDKBIN: \
    NONEBIN: \

define gb_LinkTarget__get_installname
$(if $(2),-install_name '$(2)$(1)',)
endef

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_OBJCXXFLAGS) $(gb_COMPILEROPTFLAGS)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CFLAGS += -g
gb_LinkTarget_CXXFLAGS += -g
gb_LinkTarget_OBJCXXFLAGS += -g
endif

gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

# FIXME the DYLIB_FILE mess is only necessary because
# solver layout is different from installation layout
# FIXME framework handling very hackish
define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    DYLIB_FILE=`$(gb_MKTEMP) $(dir $(1))` && \
    $(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl $(3) $(patsubst lib%.dylib,-l%,$(foreach lib,$(4),$(call gb_Library_get_filename,$(lib)))) > $${DYLIB_FILE} && \
    $(gb_CXX) \
        $(3) \
        $(patsubst lib%.dylib,-l%,$(foreach lib,$(filter-out $(gb_Library__FRAMEWORKS),$(4)),$(call gb_Library_get_filename,$(lib)))) \
        $(addprefix -framework ,$(filter $(gb_Library__FRAMEWORKS),$(4))) \
        $(foreach object,$(6),$(call gb_CObject_get_target,$(object))) \
        $(foreach object,$(7),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(8),$(call gb_ObjCxxObject_get_target,$(object))) \
        $(foreach lib,$(5),$(call gb_StaticLibrary_get_target,$(lib))) \
        -o $(1) \
        `cat $${DYLIB_FILE}` && \
    $(if $(filter shl exe,$(TARGETTYPE)),$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl $(TARGETTYPE) $(LAYER) $(1) &&,) \
    $(if $(filter shl,$(TARGETTYPE)),macosx-create-bundle $(1) &&,) \
    rm -f $${DYLIB_FILE})
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -dynamiclib -single_module
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .dylib
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_gcc$(gb_Library_PLAINEXT)
else
gb_Library_STLEXT := port_gcc_stldebug$(gb_Library_PLAINEXT)
endif

ifeq ($(CPUNAME),INTEL)
gb_Library_OOOEXT := mxi$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
else # ifeq ($(CPUNAME),POWERPC)
gb_Library_OOOEXT := mxp$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
endif

gb_Library__FRAMEWORKS += \
    Cocoa \

gb_Library_PLAINLIBS_NONE += \
    Cocoa \
    objc \
    jpeg \
    m \
    pthread \
    z \

gb_Library_FILENAMES := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
    $(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
    $(foreach lib,$(gb_Library_STLLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_STLEXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \


# HACK
# SUBSTITING OOOLIB with OOO to make the perl script happy
define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETTYPE := shl
$(call gb_LinkTarget_get_target,$(2)) : LAYER :=$(subst OOOLIB,OOO,$(call gb_Library_get_layer,$(1)))

endef

define gb_Library_get_rpath
$(call gb_LinkTarget__get_installname,$(call gb_Library_get_filename,$(1)),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

gb_Library_LAYER := \
    $(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTLIBS),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_STLLIBS),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
    $(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOOLIB) \
    $(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS := -static -nostdlib
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
    $(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
    $(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETTYPE := staticlib
$(call gb_LinkTarget_get_target,$(2)) : LAYER :=

endef

# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS := -bind_at_load

# HACK
# SUBSTITING OOOLIB with OOO to make the perl script happy
define gb_Executable_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : TARGETTYPE := exe
$(call gb_LinkTarget_get_target,$(2)) : LAYER :=$(subst OOOLIB,OOO,$(call gb_Executable_get_layer,$(1)))

endef

define gb_Executable_get_rpath
$(call gb_LinkTarget__get_installname,$(1),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1))))
endef

gb_Executable_LAYER := \
    $(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
    $(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
    $(foreach exe,$(gb_Executable_OOO),$(exe):OOOLIB) \
    $(foreach exe,$(gb_Executable_BRAND),$(exe):BRAND) \
    $(foreach exe,$(gb_Executable_NONE),$(exe):NONEBIN) \


# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartMergeTarget

gb_SrsPartMergeTarget_TRANSEXPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc
gb_SrsPartTarget_RSCCOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

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

gb_ComponentTarget_XSLTPROCPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib
gb_ComponentTarget_PREFIXBASISNATIVE := vnd.sun.star.expand:$$OOO_BASE_DIR/program/

# vim: set noet sw=4 ts=4:
