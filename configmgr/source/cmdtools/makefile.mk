#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:42:27 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

