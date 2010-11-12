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
COM := C52

gb_MKTEMP := mktemp -p
gb_AWK := nawk

gb_CC := cc
gb_CXX := CC
gb_GCCP := cc

gb_OSDEFS := \
    -D$(OS) \
    -D$(GUI) \
    -DSYSV \
    -DSUN \
    -DSUN4 \
    -D_REENTRANT \
    -D_POSIX_PTHREAD_SEMANTICS \
    -D_PTHREADS \
    -DUNIX \
#	$(PTHREAD_CFLAGS) \

gb_COMPILERDEFS := \
    -D$(COM) \
#	-D$(CVER) \
#	-DCVER=$(CVER) \

ifeq ($(CPUNAME),INTEL)
gb_CPUDEFS :=
else # ifeq ($(CPUNAME),SPARC)
gb_CPUDEFS := -D__sparcv8plus
endif

gb_CFLAGS := \
    -temp=/tmp \
    -KPIC \
    -mt \
    -xCC \
    -xc99=none \

gb_CXXFLAGS := \
    -temp=/tmp \
    -KPIC \
    -mt \
    -features=no%altspell \
    -library=no%Cstd \
    +w2 \
    -erroff=doubunder,identexpected,inllargeuse,inllargeint,notemsource,reftotemp,truncwarn,wnoretvalue,anonnotype \

gb_CFLAGS_WERROR := \
    -errwarn=%all \

gb_CXXFLAGS_WERROR := \
    -xwe \

gb_LinkTarget_EXCEPTIONFLAGS := \
    -DEXCEPTIONS_ON \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
    -DEXCEPTIONS_OFF \
    -noex \

gb_LinkTarget_LDFLAGS := \
    $(subst -L../lib , ,$(SOLARLIB)) \
    -temp=/tmp \
    -w \
    -mt \
    -z combreloc \
    -norunpath \
    -PIC \
    -library=no%Cstd \

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS :=
else
ifeq ($(CPUNAME),INTEL)
gb_COMPILEROPTFLAGS := -xarch=generic -xO3
else # ifeq ($(CPUNAME),SPARC)
#  -m32 -xarch=sparc        restrict target to 32 bit sparc
#  -xO3                     optimization level 3
#  -xspace                  don't do optimizations which do increase binary size
#  -xprefetch=yes           do prefetching (helps on UltraSparc III)
gb_COMPILEROPTFLAGS := -m32 -xarch=sparc -xO3 -xspace -xprefetch=yes
endif
endif


# Helper class

gb_Helper_abbreviate_dirs_native = $(gb_Helper_abbreviate_dirs)
gb_Helper_GNUCOPY := gnucp


# CObject class

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    rm -f $(call gb_CObject_get_dep_target,$(2)) && \
    mkdir -p $(dir $(call gb_CObject_get_dep_target,$(2))) && \
    $(gb_CC) \
        -c $(3) \
        -o $(1) \
        -xMMD \
        -xMF $(call gb_CObject_get_dep_target,$(2)) \
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
        -xMMD \
        -xMF $(call gb_CxxObject_get_dep_target,$(2)) \
        $(4) $(5) \
        -I$(dir $(3)) \
        $(6))
endef


# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef
define gb_LinkTarget_get_rpath
-R\'$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_LinkTarget_get_layer,$(2)))\'
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
gb_LinkTarget_CXXFLAGS += -g
gb_LinkTarget_CFLAGS += -g
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
        $(foreach lib,$(5),$(call gb_StaticLibrary_get_target,$(lib))) \
        -o $(1))
endef


# Library class

#gb_Library_DEFS := -DSHAREDLIB -D_DLL_
gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -Bdynamic -z text -G
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .so
gb_Library_RTEXT := C52$(gb_Library_PLAINEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_sunpro$(gb_Library_PLAINEXT)
else
gb_Library_STLEXT := port_sunpro_debug$(gb_Library_PLAINEXT)
endif

ifeq ($(CPUNAME),INTEL)
gb_Library_OOOEXT := si$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
else # ifeq ($(CPUNAME),SPARC)
gb_Library_OOOEXT := ss$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
endif

gb_Library_PLAINLIBS_NONE += \
    dl \
    jpeg \
    m \
    pthread \
    X11 \
    z

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


gb_Library__Library_platform =


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS := \
    -Bstatic \
    -xar \

gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
    $(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
    $(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \


# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable__Executable_platform =


# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib


# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc
gb_SrsPartTarget_RSCCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

# Sun cc/CC support -xM1/-xMF flags, but unfortunately refuse input files that
# do not have the right suffix, so use makedepend here...
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
    > $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef


# ComponentTarget

gb_ComponentTarget_XSLTPROCPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_ComponentTarget_PREFIXBASISNATIVE := vnd.sun.star.expand:$$OOO_BASE_DIR/program/

# vim: set noet sw=4 ts=4:

