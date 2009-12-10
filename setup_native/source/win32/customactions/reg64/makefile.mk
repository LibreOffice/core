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
# $Revision: 1.20 $
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

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=reg64msi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE 

.INCLUDE : settings.mk

.IF "$(USE_SYSTEM_STL)" != "YES"
CFLAGS+=-D_STLP_USE_STATIC_LIB
.ENDIF

#Disable precompiled header
CDEFS+=-Dnot_used_define_to_disable_pch

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

UWINAPILIB=

SLOFILES = \
    $(SLO)$/reg64.obj

STDSHL += \
    $(KERNEL32LIB)\
    $(USER32LIB)\
    $(ADVAPI32LIB)\
    $(SHELL32LIB)\
    $(MSILIB)\
    $(SHLWAPILIB)\
    
#	msvcprt.lib 							

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1OBJS=$(SLOFILES) 
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF


# --- mapimailer --------------------------------------------------------

#TARGETTYPE=CUI

#OBJFILES=   $(OBJ)$/reg64.obj

#APP1TARGET=reg64
#APP1OBJS=$(OBJFILES)
#APP1STDLIBS=$(KERNEL32LIB)\
#	$(ADVAPI32LIB)\
#	$(MSILIB)\
#   $(SHELL32LIB)\
#    msvcprt.lib\
#    $(OLE32LIB)\
#    $(COMCTL32LIB)\
#    $(UUIDLIB)

            
#APP1DEF=$(MISC)$/$(APP1TARGET).def 

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))
.EXPORT : INCLUDE

# -------------------------------------------------------------------------


