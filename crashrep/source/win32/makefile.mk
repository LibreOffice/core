#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 12:40:24 $
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

#SHL1STDLIBS=$(SALLIB)

STDLIB1=\
    $(SALLIB)\
    $(GDI32LIB)\
    $(COMCTL32LIB)\
    $(COMDLG32LIB)\
    $(ADVAPI32LIB)\
    $(WSOCK32LIB)\
    $(SHELL32LIB)\
    $(DBGHELPLIB)

APP1NOSVRES=$(RES)$/$(TARGET).res

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/soreport.obj: $(INCCOM)$/_version.h

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(LNGFILES) makefile.mk rcfooter.txt rcheader.txt rctemplate.txt ctrylnglist.txt
    lngconvex.exe -ulf $(ULFDIR)$/crashrep.ulf -rc $(RES)$/$(TARGET).rc  -rct rctemplate.txt -rch rcheader.txt -rcf rcfooter.txt
    
