#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 15:16:23 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

RCFILES=$(TARGET).rc

# --- Files --------------------------------------------------------

OBJFILES=\
    $(OBJ)$/soreport.obj\
    $(OBJ)$/base64.obj
    
LNGFILES=..$/all$/crashrep.lng

APP1OBJS=$(OBJFILES)

APP1NOSAL=TRUE
                        
APP1TARGET=$(TARGET)

#SHL1STDLIBS=$(SALLIB)

STDLIB1=\
    $(SALLIB)\
    gdi32.lib\
    comctl32.lib\
    comdlg32.lib\
    advapi32.lib\
    wsock32.lib\
    dbghelp.lib

APP1NOSVRES=$(RES)$/$(TARGET).res

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/soreport.obj: $(INCCOM)$/_version.h

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(LNGFILES) makefile.mk rcfooter.txt rcheader.txt rctemplate.txt ctrylnglist.txt
    +lngconvex.exe -lng ..\all\crashrep.lng -rc $(TARGET).rc -c  ctrylnglist.txt  -rct rctemplate.txt -rch rcheader.txt -rcf rcfooter.txt
    
