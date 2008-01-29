#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 14:42:04 $
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
PRJNAME=lingucomponent

TARGET=guesslang

ENABLE_EXCEPTIONS=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)$(COM)"=="WNTGCC"
LIBTEXTCATLIB=-ltextcat
.ELSE               # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
LIBTEXTCATLIB=ilibtextcat.lib
.ENDIF  # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

SLOFILES = \
        $(SLO)$/altstrfunc.obj \
        $(SLO)$/guess.obj \
        $(SLO)$/guesslang.obj \
        $(SLO)$/simpleguesser.obj


SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)        \
        $(CPPULIB)              \
        $(LIBTEXTCATLIB)        \
        $(SALLIB)               \
        $(SVLLIB)               \
        $(SVTOOLLIB)            \
        $(TOOLSLIB)             \
        $(UNOTOOLSLIB)

# build DLL
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
.IF "$(OS)"!="MACOSX"
SHL1VERSIONMAP= $(TARGET).map
.ENDIF

# build DEF file
DEF1NAME    =$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

