#**************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:39:02 $
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
#**************************************************************************

PRJ=..$/..
PRJNAME=scaddins

TARGET=date

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/types.rdb $(BIN)$/dateadd.rdb
UNOUCRDEP=$(UNOUCRRDB)

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
NO_OFFUH=TRUE
CPPUMAKERFLAGS*=-C

UNOTYPES=\
    com.sun.star.sheet.addin.XDateFunctions \
    com.sun.star.sheet.addin.XMiscFunctions \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XSingleComponentFactory \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.sheet.XAddIn \
    com.sun.star.sheet.XCompatibilityNames \
    com.sun.star.lang.XServiceName \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.uno.XWeak \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XComponentContext \
    com.sun.star.util.Date

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/datefunc.obj

ALLIDLFILES=\
    dateadd.idl

SRS1NAME=$(TARGET)
SRC1FILES =\
    datefunc.src

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(VCLLIB)			\
        $(TOOLSLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB)

SHL1DEPN=makefile.mk

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/date.srs

RESLIB1NAME=date
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(BIN)$/dateadd.rdb: $(ALLIDLFILES)
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@


