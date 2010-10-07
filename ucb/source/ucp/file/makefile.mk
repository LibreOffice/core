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
PRJNAME=ucb
TARGET=ucpfile
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFILE_MAJOR=1

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/prov.obj      \
    $(SLO)$/bc.obj        \
    $(SLO)$/shell.obj     \
    $(SLO)$/filtask.obj   \
    $(SLO)$/filrow.obj    \
    $(SLO)$/filrset.obj   \
    $(SLO)$/filid.obj     \
    $(SLO)$/filnot.obj    \
    $(SLO)$/filprp.obj    \
    $(SLO)$/filinpstr.obj \
    $(SLO)$/filstr.obj    \
    $(SLO)$/filcmd.obj    \
    $(SLO)$/filglob.obj   \
    $(SLO)$/filinsreq.obj \
    $(SLO)$/filrec.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(UCPFILE_MAJOR)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.IF "$(GUI)" == "OS2"
DEF1EXPORTFILE=exports2.dxp
.ENDIF

DEF1NAME=$(SHL1TARGET)
.ENDIF # L10N_framework

.INCLUDE: target.mk


ALLTAR : $(MISC)/ucpfile1.component

$(MISC)/ucpfile1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpfile1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpfile1.component
