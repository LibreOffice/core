#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
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
APP1STDLIBS+= ole32.lib oleaut32.lib uuid.lib shell32.lib advapi32.lib libci.lib msvcirt.lib
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

