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

PRJNAME=crashrep
TARGET=soreport
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Resources ----------------------------------------------------

RCFILES=$(RES)$/$(TARGET).rc

# --- Files --------------------------------------------------------

OBJFILES=\
    $(OBJ)$/soreport.obj\
    $(OBJ)$/base64.obj
    
.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/crash_res
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=..$/all
.ENDIF			# "$(WITH_LANG)"!=""
LNGFILES=$(ULFDIR)$/crashrep.ulf

APP1OBJS=$(OBJFILES)

APP1NOSAL=TRUE
                        
APP1TARGET=$(TARGET)
APP1RPATH=BRAND

STDLIB1=\
    $(SALLIB)\
    $(GDI32LIB)\
    $(COMCTL32LIB)\
    $(COMDLG32LIB)\
    $(ADVAPI32LIB)\
    $(WS2_32LIB)\
    $(SHELL32LIB)\
    $(DBGHELPLIB)\
    psapi.lib

APP1NOSVRES=$(RES)$/$(TARGET).res

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/soreport.obj: $(INCCOM)$/_version.h

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(LNGFILES) makefile.mk rcfooter.txt rcheader.txt rctemplate.txt ctrylnglist.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/crashrep.ulf -rc $(RES)$/$(TARGET).rc  -rct rctemplate.txt -rch rcheader.txt -rcf rcfooter.txt
    
