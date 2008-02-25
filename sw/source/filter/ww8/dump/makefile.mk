#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:54:06 $
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

# --- Settings ---------------------------------------------------

.INCLUDE :	settings.mk

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
APP1STDLIBS= $(SVTOOLLIB) $(SVLLIB) $(TOOLSLIB) $(SVLIB) $(SVMEMLIB) $(SO2LIB)
# irtl.lib
.IF "$(SO3)" == ""
APP1STDLIBS+= sdstor.lib
.ELSE
APP1STDLIBS+= $(SOTLIB)
.ENDIF
.IF "$(COMEX)"=="10"
APP1STDLIBS+= $(OLE32LIB) $(OLEAUT32LIB) $(UUIDLIB) $(SHELL32LIB) $(ADVAPI32LIB) libci.lib
.ELSE
APP1STDLIBS+= $(OLE32LIB) $(OLEAUT32LIB) $(UUIDLIB) $(SHELL32LIB) $(ADVAPI32LIB) libci.lib msvcirt.lib
.ENDIF
.ELSE
.IF "$(GUI)" == "OS2"
APP1STDLIBS=sdstor.lib $(SO2LIB) $(SVTOOLLIB) $(SVLIB) tools.lib $(SVMEMLIB) $(SVXLIB)
.ELSE
APP1STDLIBS=$(SVTOOLLIB) tools.lib $(SVLIB) $(SVMEMLIB) sdstor.lib $(SO2LIB) $(SVXLIB)
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

