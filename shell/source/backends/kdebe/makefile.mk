#*************************************************************************
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
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=shell
TARGET=kdebe

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# For some of the included external KDE headers, GCC complains about shadowed
# symbols in instantiated template code only at the end of a compilation unit,
# so the only solution is to disable that warning here:
.IF "$(COM)" == "GCC"
CFLAGSCXX+=-Wno-shadow
.ENDIF

# no "lib" prefix
DLLPRE =

.IF "$(ENABLE_KDE)" == "TRUE"

CFLAGS+=$(KDE_CFLAGS)

# --- Files ---

SLOFILES=\
    $(SLO)$/kdeaccess.obj \
    $(SLO)$/kdebackend.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)1.uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1LINKFLAGS+=$(KDE_LIBS) -lkio
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF          # "$(ENABLE_KDE)" == "TRUE"

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MISC)/kdebe1.component

$(MISC)/kdebe1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        kdebe1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt kdebe1.component
