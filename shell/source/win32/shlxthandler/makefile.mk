#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:16:13 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
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
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
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

PRJ=..$/..$/..
PRJNAME=shell
TARGET=shlxthdl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

TARGET1=regsvrex
TARGETTYPE=CUI


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# set the define _WINXPSDK if you compile
# with a Microsoft platform sdk for Windows
# 2000 and greater
#
# set ISOLATION_AWARE_ENABLE for activating
# the Windows XP visual styles

#-D_WINXPSDK set this for a Win2000/WinXP Platform SDK
#-DBUILD_SOSL for extended functionality (Infotip and
# Column Handler)

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 

# --- Files --------------------------------------------------------

RCFILES=$(TARGET).rc

LNGFILES=$(TARGET).lng

SLOFILES=$(SLO)$/classfactory.obj\
    $(SLO)$/columnprovider.obj\
    $(SLO)$/dbgmacros.obj\
    $(SLO)$/fileextensions.obj\
    $(SLO)$/infotip.obj\
    $(SLO)$/propshthdl.obj\
    $(SLO)$/registry.obj\
    $(SLO)$/saxdochdl.obj\
    $(SLO)$/saxprsrexcptn.obj\
    $(SLO)$/shlxthdl.obj\
    $(SLO)$/xmlprsr.obj\
    $(SLO)$/ziparchv.obj\
    $(SLO)$/zipexcptn.obj\
    $(SLO)$/metaaccessor.obj\
    $(SLO)$/utilities.obj\
    $(SLO)$/listviewbuilder.obj\
    $(SLO)$/document_statistic.obj\
    $(SLO)$/iso8601_converter.obj
    
SHL1TARGET=$(TARGET)

SHL1STDLIBS=uwinapi.lib\
    oleaut32.lib\
    advapi32.lib\
    ole32.lib\
    uuid.lib\
    shell32.lib\
    comctl32.lib\
    gdi32.lib
    
SHL1LIBS=$(SOLARLIBDIR)$/zlib.lib\
    $(SOLARLIBDIR)$/expat_xmlparse.lib\
    $(SOLARLIBDIR)$/expat_xmltok.lib

SHL1DEPN=

SHL1OBJS=$(SLOFILES) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1RES=$(RES)$/$(TARGET).res

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

OBJFILES=$(OBJ)$/regsvrex.obj
APP1TARGET=$(TARGET1)
APP1OBJS=$(OBJFILES)
APP1STDLIBS=kernel32.lib
APP1DEF=$(MISC)$/$(APP1TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(LNGFILES) makefile.mk rcfooter.txt rcheader.txt rctmpl.txt ctrylnglist.txt
    +$(BIN)$/lngconvex.exe -lng shlxthdl.lng -rc shlxthdl.rc -c  ctrylnglist.txt  -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt
    
