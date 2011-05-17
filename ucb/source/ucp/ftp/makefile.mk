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
TARGET=ucpftp
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFTP_MAJOR=1


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------
.IF "$(L10N_framework)"==""
# first target ( shared library )

SLOFILES1=\
    $(SLO)$/ftpservices.obj  \
    $(SLO)$/ftpcontentprovider.obj  \
    $(SLO)$/ftpcontent.obj   \
    $(SLO)$/ftpcontentidentifier.obj   \
    $(SLO)$/ftpcontentcaps.obj \
    $(SLO)$/ftpdynresultset.obj  \
    $(SLO)$/ftpresultsetbase.obj \
    $(SLO)$/ftpresultsetI.obj \
    $(SLO)$/ftploaderthread.obj  \
    $(SLO)$/ftpinpstr.obj	\
    $(SLO)$/ftpdirp.obj     \
    $(SLO)$/ftpcfunc.obj     \
    $(SLO)$/ftpurl.obj     \
    $(SLO)$/ftpintreq.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES1)

# --- Shared-Library 1 ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCPFTP_MAJOR)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB) \
    $(CURLLIB)

.IF "$(GUI)" == "OS2"
SHL1STDLIBS+=ssl.lib crypto.lib libz.lib
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS= \
    $(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework
.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpftp1.component

$(MISC)/ucpftp1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpftp1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpftp1.component
