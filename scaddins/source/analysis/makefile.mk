#**************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#**************************************************************************

PRJ=..$/..
PRJNAME=scaddins

TARGET=analysis


ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/types.rdb $(BIN)$/analysisadd.rdb
UNOUCRDEP=$(UNOUCRRDB)

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
NO_OFFUH=TRUE
CPPUMAKERFLAGS*=-C

UNOTYPES=\
    com.sun.star.sheet.addin.XAnalysis \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XSingleComponentFactory \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.sheet.LocalizedName \
    com.sun.star.sheet.XAddIn \
    com.sun.star.sheet.XCompatibilityNames \
    com.sun.star.lang.XServiceName \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.uno.XWeak \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XComponentContext \
    com.sun.star.util.Date \
    com.sun.star.util.XNumberFormatter \
    com.sun.star.util.XNumberFormatTypes \
    com.sun.star.sheet.NoConvergenceException

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/analysis.obj		\
    $(SLO)$/financial.obj		\
    $(SLO)$/analysishelper.obj	\
    $(SLO)$/bessel.obj

ALLIDLFILES=\
    analysisadd.idl

SRS1NAME=$(TARGET)
SRC1FILES =  \
        analysis.src		\
        analysis_funcnames.src	\
        analysis_deffuncnames.src

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(TOOLSLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1DEPN=makefile.mk

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/analysis.srs

RESLIB1NAME=analysis
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(BIN)$/analysisadd.rdb: $(ALLIDLFILES)
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@


