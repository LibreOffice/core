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
# $Revision: 1.12 $
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
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=configtools
TARGET1=configimport
TARGET2=setofficelang

# Targettype should be CUI, but we need to be able to run the tools on Windows without a shell
.IF "$(GUI)" == "WNT"
TARGETTYPE=GUI
APP1NOSAL="TRUE"
APP2NOSAL="TRUE"
.ELSE  # "$(GUI)" == "WNT
TARGETTYPE=CUI
.ENDIF # "$(GUI)" == "WNT

LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APPSTDLIBS=\
            $(SALLIB) \
            $(CPPULIB)	\
            $(CPPUHELPERLIB)

OBJFILES=\
    $(OBJ)$/configimport.obj \
    $(OBJ)$/setofficelang.obj \
    $(OBJ)$/unomain.obj 


# --- Apps --------------------------------------------------------
APP1STDLIBS = $(APPSTDLIBS)
APP2STDLIBS = $(APPSTDLIBS)

.IF "$(GUI)"=="UNX"
APP1TARGET= $(TARGET1).bin
APP2TARGET= $(TARGET2).bin
.ELSE
APP1TARGET= $(TARGET1)
APP2TARGET= $(TARGET2)
.ENDIF

APP1OBJS=	\
    $(OBJ)$/filehelper.obj \
    $(OBJ)$/oslstream.obj \
    $(OBJ)$/bufferedfile.obj \
    $(OBJ)$/configimport.obj \
    $(OBJ)$/unomain.obj 

APP2OBJS=	\
    $(OBJ)$/setofficelang.obj \
    $(OBJ)$/unomain.obj 

# --- Scripts ------------------------------------------------------

UNIXTEXT = $(MISC)/configimport.sh

.INCLUDE :  target.mk

