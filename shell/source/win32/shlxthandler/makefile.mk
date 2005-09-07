#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:58:57 $
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

PRJ=..$/..$/..
PRJNAME=shell
TARGET=shlxthdl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-D_WIN32_IE=0x501

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/classfactory.obj\
    $(SLO)$/columninfo.obj\
    $(SLO)$/infotips.obj\
    $(SLO)$/propsheets.obj\
    $(SLO)$/shlxthdl.obj\
    $(SLO)$/listviewbuilder.obj\
    $(SLO)$/document_statistic.obj\
    $(SLO)$/thumbviewer.obj
       
SHL1TARGET=$(TARGET)

SHL1STDLIBS=uwinapi.lib\
    oleaut32.lib\
    advapi32.lib\
    ole32.lib\
    uuid.lib\
    shell32.lib\
    kernel32.lib\
    gdi32.lib\
    gdiplus.lib
    
SHL1LIBS=$(SOLARLIBDIR)$/zlib.lib\
    $(SOLARLIBDIR)$/expat_xmlparse.lib\
    $(SOLARLIBDIR)$/expat_xmltok.lib\
    $(SLB)$/util.lib\
    $(SLB)$/ooofilereader.lib
    
SHL1DEPN=

SHL1OBJS=$(SLOFILES) 
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1RES=$(RES)$/$(TARGET).res

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

    
