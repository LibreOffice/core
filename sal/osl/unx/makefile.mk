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
.IF "$(WORK_STAMP)"=="MIX364"
TARGET=cppsal
.ELSE
TARGET=cpposl
.ENDIF
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES= \
            $(SLO)$/conditn.obj  \
            $(SLO)$/diagnose.obj \
            $(SLO)$/semaphor.obj \
            $(SLO)$/socket.obj   \
            $(SLO)$/interlck.obj \
            $(SLO)$/mutex.obj    \
            $(SLO)$/nlsupport.obj \
            $(SLO)$/thread.obj   \
            $(SLO)$/module.obj   \
            $(SLO)$/process.obj  \
            $(SLO)$/security.obj \
            $(SLO)$/profile.obj  \
            $(SLO)$/time.obj     \
            $(SLO)$/signal.obj   \
            $(SLO)$/pipe.obj   	 \
            $(SLO)$/system.obj	 \
            $(SLO)$/util.obj	 \
            $(SLO)$/tempfile.obj\
            $(SLO)$/file.obj     \
            $(SLO)$/file_misc.obj\
            $(SLO)$/file_url.obj\
            $(SLO)$/file_error_transl.obj\
            $(SLO)$/file_path_helper.obj\
            $(SLO)$/file_stat.obj \
            $(SLO)$/file_volume.obj \
            $(SLO)$/uunxapi.obj\
            $(SLO)$/process_impl.obj\
            $(SLO)$/salinit.obj

OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/semaphor.obj \
            $(OBJ)$/socket.obj   \
            $(OBJ)$/interlck.obj \
            $(OBJ)$/mutex.obj    \
            $(OBJ)$/nlsupport.obj \
            $(OBJ)$/thread.obj   \
            $(OBJ)$/module.obj   \
            $(OBJ)$/process.obj  \
            $(OBJ)$/security.obj \
            $(OBJ)$/profile.obj  \
            $(OBJ)$/time.obj     \
            $(OBJ)$/signal.obj   \
            $(OBJ)$/pipe.obj   	 \
            $(OBJ)$/system.obj	 \
            $(OBJ)$/util.obj	 \
            $(OBJ)$/tempfile.obj\
            $(OBJ)$/file.obj     \
            $(OBJ)$/file_misc.obj\
            $(OBJ)$/file_url.obj\
            $(OBJ)$/file_error_transl.obj\
            $(OBJ)$/file_path_helper.obj\
            $(OBJ)$/file_stat.obj \
            $(OBJ)$/file_volume.obj \
            $(OBJ)$/uunxapi.obj\
            $(OBJ)$/process_impl.obj\
            $(OBJ)$/salinit.obj
            

.IF "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/osxlocale.obj
.ENDIF

.IF "$(OS)"=="SOLARIS" || "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || \
    "$(OS)$(CPU)"=="LINUXS" || "$(OS)"=="MACOSX" || "$(OS)"=="AIX" || \
    "$(OS)"=="OPENBSD" || "$(OS)"=="DRAGONFLY"
SLOFILES += $(SLO)$/backtrace.obj
OBJFILES += $(OBJ)$/backtrace.obj
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(COM)"=="C50"
APP1STDLIBS+=-lC
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""
CFLAGS+=-DSAL_ENABLE_CRASH_REPORT
.ENDIF

.INCLUDE :  target.mk

.IF "$(OS)$(CPU)"=="SOLARISU" || "$(OS)$(CPU)"=="SOLARISS" || "$(OS)$(CPU)"=="NETBSDS" || "$(OS)$(CPU)"=="LINUXS"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
     touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
     touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_sparc.s
    $(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_sparc.s
    $(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_sparc.s: asm/interlck_sparc.s
    tr -d "\015" < $< > $@

.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISI"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
    touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
    touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_x86.s
    $(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_x86.s
    $(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_x86.s: asm/interlck_x86.s
    tr -d "\015" < $< > $@

.ENDIF
