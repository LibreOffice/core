#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 17:34:43 $
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
TARGET=sysshell
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(OS)" != "MACOSX"

LIB1OBJFILES=$(SLO)$/systemshell.obj
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1ARCHIV=$(SLB)$/lib$(TARGET).a

SLOFILES=$(SLO)$/recently_used_file.obj \
         $(SLO)$/recently_used_file_handler.obj

SHL1TARGET=recentfile
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

.ELSE
#SHL1IMPLIB=
.ENDIF

SHL1STDLIBS=$(EXPATASCII3RDLIB)\
    $(SALLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB)\
    $(COMPHELPERLIB)

SHL1LIBS=$(SLB)$/xmlparser.lib
SHL1OBJS=$(SLOFILES)
SHL1VERSIONMAP=recfile.map

.ELSE

dummy:
    @echo "Nothing to build for MACOSX"

.ENDIF # MACOSX
# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
