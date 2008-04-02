#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2008-04-02 09:43:10 $
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
TARGET=ooofilereader
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)" != "OS2"
CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-D_WIN32_IE=0x501
CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS_X64+=-D_WIN32_IE=0x501
.ENDIF
.IF "$(GUI)" == "OS2"
CFLAGS+=-DISOLATION_AWARE_ENABLED -DXML_UNICODE -DXML_UNICODE_WCHAR_T -DUNICODE -D_UNICODE
.ENDIF

# --- Files --------------------------------------------------------


SLOFILES=$(SLO)$/basereader.obj\
         $(SLO)$/metainforeader.obj\
         $(SLO)$/contentreader.obj\
         $(SLO)$/simpletag.obj\
         $(SLO)$/keywordstag.obj\
         $(SLO)$/autostyletag.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)
LIB1FILES=$(SLB)$/zipfile.lib\
          $(SLB)$/xmlparser.lib

.IF "$(BUILD_X64)"!=""
SLOFILES_X64=$(SLO_X64)$/basereader.obj\
         $(SLO_X64)$/metainforeader.obj\
         $(SLO_X64)$/contentreader.obj\
         $(SLO_X64)$/simpletag.obj\
         $(SLO_X64)$/keywordstag.obj\
         $(SLO_X64)$/autostyletag.obj

LIB1TARGET_X64=$(SLB_X64)$/$(TARGET).lib
LIB1OBJFILES_X64=$(SLOFILES_X64)
LIB1FILES_X64=$(SLB_X64)$/zipfile.lib\
          $(SLB_X64)$/xmlparser.lib
.ENDIF # "$(BUILD_X64)"!=""
# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_wntx64.mk

