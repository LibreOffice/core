#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:33:28 $
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

PRJNAME=extensions
TARGET=pl
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_MOZILLA)" != "NO"

LIB1TARGET = $(SLB)$/plall.lib
LIB1FILES  = \
    $(SLB)$/plbase.lib	\
    $(SHL1LINKLIB)

.IF "$(GUI)" == "UNX"
.IF "$(GUIBASE)"=="aqua"
.IF "$(WITH_MOZILLA)"=="YES"
SHL1LINKLIB = $(SLB)$/plaqua.lib
.ENDIF
.ELSE
SHL1LINKLIB = $(SLB)$/plunx.lib
.ENDIF # $(GUIBASE)==aqua
.IF "$(OS)" == "SOLARIS"
SHL1OWNLIBS = -lsocket
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)" == "WNT"
SHL1LINKLIB = $(SLB)$/plwin.lib
SHL1OWNLIBS = \
    $(VERSIONLIB)	\
    $(OLE32LIB)	\
    $(ADVAPI32LIB)
.ENDIF # WNT

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= \
    $(LIBSTLPORT)		\
    $(TKLIB)
.ELSE
SHL1STDLIBS= \
    $(TKLIB)
.ENDIF

SHL1STDLIBS+= \
    $(VCLLIB)			\
    $(SVLLIB)			\
    $(TOOLSLIB)			\
    $(VOSLIB)			\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)

.IF "$(OS)"=="MACOSX" && "$(GUIBASE)"=="unx"
SHL1STDLIBS+= -lX11
.ENDIF

SHL1STDLIBS+=$(SHL1OWNLIBS)

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



