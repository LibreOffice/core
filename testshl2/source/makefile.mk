#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9.2.1 $
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
PRJ=..

PRJNAME=testshl2
TARGET=testshl2
TARGETTYPE=CUI
PRJINC=.

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------

COMMON_OBJFILES=\
    $(OBJ)$/getopt.obj \
    $(OBJ)$/filehelper.obj \
    $(OBJ)$/dynamicregister.obj 

TESTSHL_OBJFILES=\
    $(OBJ)$/autoregisterhelper.obj \
    $(OBJ)$/testshl.obj 

VERSIONINFO_OBJFILES=\
    $(OBJ)$/versionhelper.obj \
    $(OBJ)$/versioner.obj 

BMSMA_OBJFILES=\
    $(OBJ)$/bmsma.obj

# for right dependencies
# OBJFILES= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES) $(VERSIONINFO_OBJFILES)

# ------------------------------------------------------------------
APP1TARGET= $(TARGET)
APP1OBJS= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES)
APP1RPATH=NONE

APP1STDLIBS=$(SALLIB)

# the c5t*.lib files are out of the cppunit module
# APP1DEPN= \
# 	$(SOLARLIBDIR)$/c5t_no_regallfkt.lib \
# 	$(SOLARLIBDIR)$/c5t_testresult.lib

.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "GCC"
APP1STDLIBS+= \
    $(SOLARLIBDIR)$/libc5t_testresult$(DLLPOSTFIX).a \
    $(SOLARLIBDIR)$/libc5t_no_regallfkt$(DLLPOSTFIX).a \
    $(SOLARLIBDIR)$/libc5t_winstuff$(DLLPOSTFIX).a
.ELSE
APP1LIBS= \
    $(SOLARLIBDIR)$/c5t_no_regallfkt.lib \
    $(SOLARLIBDIR)$/c5t_testresult.lib
APP1LIBS += $(SOLARLIBDIR)$/c5t_winstuff.lib
.ENDIF
.ENDIF

.IF "$(GUI)" == "OS2"
APP1LIBS= \
    $(SOLARLIBDIR)$/c5t_no_regallfkt.lib \
    $(SOLARLIBDIR)$/c5t_testresult.lib
.ENDIF

.IF "$(GUI)" == "UNX"
APP1STDLIBS+= \
    $(SOLARLIBDIR)$/libc5t_testresult$(DLLPOSTFIX).a \
    $(SOLARLIBDIR)$/libc5t_no_regallfkt$(DLLPOSTFIX).a
.ENDIF

#
# ------------------------------------------------------------------
APP2TARGET= versioninfo
APP2OBJS= $(COMMON_OBJFILES) $(VERSIONINFO_OBJFILES)

APP2STDLIBS=$(SALLIB)

APP2DEPN= 
APP2LIBS= 

# ------------------------------------------------------------------
APP3TARGET= $(TARGET)_timeout
APP3OBJS= 	$(OBJ)$/getopt.obj $(OBJ)$/filehelper.obj $(OBJ)$/terminate.obj

APP3STDLIBS=$(SALLIB)

APP3DEPN= 
APP3LIBS= 

.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "GCC"
APP3STDLIBS+= \
    $(SOLARLIBDIR)$/libc5t_winstuff$(DLLPOSTFIX).a
.ELSE
APP3LIBS += $(SOLARLIBDIR)$/c5t_winstuff.lib
.ENDIF
.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
