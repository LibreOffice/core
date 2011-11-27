#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

PRJ=..$/..

PRJNAME=sal
TARGET=cpprtl
ENABLE_EXCEPTIONS=TRUE
USE_LDUMP2=TRUE

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(VALGRIND_CFLAGS)" != ""
CFLAGS += $(VALGRIND_CFLAGS)
CDEFS  += -DHAVE_MEMCHECK_H=1
.ENDIF # VALGRIND_CFLAGS

.IF "$(ALLOC)" == "SYS_ALLOC" || "$(ALLOC)" == "TCMALLOC" || "$(ALLOC)" == "JEMALLOC"
CDEFS+= -DFORCE_SYSALLOC
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

SLOFILES=   \
            $(SLO)$/memory.obj      \
            $(SLO)$/cipher.obj      \
            $(SLO)$/crc.obj         \
            $(SLO)$/digest.obj      \
            $(SLO)$/random.obj      \
            $(SLO)$/locale.obj      \
            $(SLO)$/strimp.obj      \
            $(SLO)$/hash.obj        \
            $(SLO)$/string.obj      \
            $(SLO)$/ustring.obj     \
            $(SLO)$/strbuf.obj      \
            $(SLO)$/ustrbuf.obj     \
            $(SLO)$/uuid.obj        \
            $(SLO)$/rtl_process.obj \
            $(SLO)$/byteseq.obj     \
            $(SLO)$/uri.obj			\
            $(SLO)$/bootstrap.obj  	\
            $(SLO)$/cmdargs.obj		\
            $(SLO)$/unload.obj		\
            $(SLO)$/logfile.obj     \
            $(SLO)$/math.obj        \
            $(SLO)$/alloc_global.obj\
            $(SLO)$/alloc_cache.obj \
            $(SLO)$/alloc_arena.obj \
            $(SLO)$/alloc_fini.obj

OBJFILES=   \
            $(OBJ)$/memory.obj      \
            $(OBJ)$/cipher.obj      \
            $(OBJ)$/crc.obj         \
            $(OBJ)$/digest.obj      \
            $(OBJ)$/random.obj      \
            $(OBJ)$/locale.obj      \
            $(OBJ)$/strimp.obj      \
            $(OBJ)$/hash.obj        \
            $(OBJ)$/string.obj      \
            $(OBJ)$/ustring.obj     \
            $(OBJ)$/strbuf.obj      \
            $(OBJ)$/ustrbuf.obj     \
            $(OBJ)$/uuid.obj        \
            $(OBJ)$/rtl_process.obj \
            $(OBJ)$/byteseq.obj     \
            $(OBJ)$/uri.obj			\
            $(OBJ)$/bootstrap.obj  	\
            $(OBJ)$/cmdargs.obj		\
            $(OBJ)$/unload.obj		\
            $(OBJ)$/logfile.obj     \
            $(OBJ)$/math.obj \
            $(OBJ)$/alloc_global.obj\
            $(OBJ)$/alloc_cache.obj \
            $(OBJ)$/alloc_arena.obj \
            $(OBJ)$/alloc_fini.obj

.ENDIF

# --- Makefile snippet  --------------------------------------------

# used by e.g. update info file
BOOTSTRAPMK = $(OUT)$/inc$/rtlbootstrap.mk

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(BOOTSTRAPMK)

$(BOOTSTRAPMK) :
    (echo '#include "macro.hxx"'; echo RTL_OS:=THIS_OS; echo RTL_ARCH:=THIS_ARCH) >$(BOOTSTRAPMK).cxx
    $(CXX) -E $(CFLAGS) $(INCLUDE_C) $(CFLAGSCC) $(CFLAGSOBJ) $(CDEFS) $(CDEFSOBJ) $(CFLAGSAPPEND) $(BOOTSTRAPMK).cxx | $(GREP) '^RTL_' | $(SED) -e 's/"//g' >$@
