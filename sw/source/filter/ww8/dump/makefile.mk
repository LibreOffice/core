#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2005-11-16 13:54:06 $
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

PRJ=.

PRJNAME=DUMP8
TARGET=dump8
VERSION=$(UPD)

# --- Settings ---------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

CXXFILES=	dump8.cxx dump8a.cxx ww8dout.cxx ww8darr.cxx ww8scan.cxx msvbasic.cxx

OBJFILES=	$(OBJ)$/dump8.obj $(OBJ)$/dump8a.obj $(OBJ)$/ww8dout.obj \
               $(OBJ)$/ww8darr.obj $(OBJ)$/ww8scan.obj $(OBJ)$/msvbasic.obj


.IF "$(depend)" == ""

CFLAGS+= -DDUMP

# --- DUMP8 EXE --------------------------------------------------------

# IENV=$(IENV);..\RES

APP1TARGET=$(PRJNAME)

# RESLIB1NAME=vu
# RESLIB1SRSFILES=$(SRS)$/main.srs

APP1LIBS=$(LIB1TARGET)

.IF "$(GUI)" == "WNT"
APP1STDLIBS= $(SVTOOLLIB) $(SVLLIB) $(TOOLSLIB) $(SVLIB) svmem.lib so2.lib
# irtl.lib
.IF "$(SO3)" == ""
APP1STDLIBS+= sdstor.lib
.ELSE
APP1STDLIBS+= sot.lib
.ENDIF
.IF "$(COMEX)"=="10"
APP1STDLIBS+= ole32.lib oleaut32.lib uuid.lib shell32.lib advapi32.lib libci.lib
.ELSE
APP1STDLIBS+= ole32.lib oleaut32.lib uuid.lib shell32.lib advapi32.lib libci.lib msvcirt.lib
.ENDIF
.ELSE
.IF "$(GUI)" == "OS2"
APP1STDLIBS=sdstor.lib so2.lib svtool.lib $(SVLIB) tools.lib svmem.lib svx.lib
.ELSE
APP1STDLIBS=svtool.lib tools.lib $(SVLIB) svmem.lib sdstor.lib so2.lib svx.lib
APP1STDLIBS+= ole2 compobj storage shell
.ENDIF
.ENDIF
.IF "$(GUI)"=="WNT"
APP1DEPN=	$(L)$/svtool.lib $(L)$/itools.lib $(SVLIBDEPEND) $(L)$/svmem.lib $(L)$/so2.lib
.ELSE
APP1DEPN=	$(L)$/svtool.lib $(L)$/tools.lib $(SVLIBDEPEND) $(L)$/svmem.lib $(L)$/so2.lib
.ENDIF
APP1OBJS=	$(OBJ)$/dump8.obj $(OBJ)$/dump8a.obj $(OBJ)$/ww8dout.obj \
               $(OBJ)$/ww8darr.obj $(OBJ)$/ww8scan.obj $(OBJ)/msvbasic.obj

APP1DEF=	$(MISC)$/$(PRJNAME).def
#APP1RES=	 $(RES)$/$(PRJNAME).res
#SRSFILES=	 $(SRS)$/main.srs

#-------------------------------------------------------------------------
#								Windows 3.x
#-------------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

ALL: \
    ALLTAR

$(MISC)$/$(PRJNAME).def: makefile
    echo NAME		 VIEWER 										 >$@
    echo  DESCRIPTION	'StarView - Testprogramm'                    >>$@
    echo EXETYPE	 WINDOWS										 >>$@
    echo PROTMODE													 >>$@
    echo STUB		 'winSTUB.EXE'                                   >>$@
    echo CODE		 PRELOAD MOVEABLE DISCARDABLE					 >>$@
    echo DATA		 PRELOAD MULTIPLE MOVEABLE						 >>$@
    echo HEAPSIZE	 4096											 >>$@
    echo STACKSIZE	 30000											 >>$@

.ENDIF # GUI == WIN

#-------------------------------------------------------------------------
#										OS/2
#-------------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

ALL: \
    $(SRS)$/main.srs \
    ALLTAR

$(MISC)$/$(PRJNAME).def: makefile
    echo NAME		 VIEWER WINDOWAPI								   >$@
    echo DESCRIPTION 'Storage Viewer (C)1995 STAR DIVISION GmbH'      >>$@
    echo EXETYPE	 OS2											  >>$@
    echo PROTMODE													  >>$@
    echo STUB		 'OS2STUB.EXE'                                    >>$@
    echo CODE		 LOADONCALL 									  >>$@
    echo DATA		 PRELOAD MULTIPLE								  >>$@
    echo HEAPSIZE	 4096											  >>$@
    echo STACKSIZE	 30000											  >>$@

.ENDIF # GUI == OS2

#-------------------------------------------------------------------------
#								Windows NT
#-------------------------------------------------------------------------

# --- Targets -----------------------------------------------------------

.ENDIF

.INCLUDE :	target.mk

