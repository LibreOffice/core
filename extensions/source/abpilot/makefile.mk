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
PRJ=..$/..
PRJINC=..$/inc

PRJNAME=extensions
TARGET=abp
USE_DEFFILE=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=abp
CDEFS+=-DCOMPMOD_RESPREFIX=abp

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	\
        $(SLO)$/abpfinalpage.obj	\
        $(SLO)$/abpservices.obj		\
        $(SLO)$/abspage.obj		\
        $(SLO)$/abspilot.obj		\
        $(SLO)$/admininvokationimpl.obj	\
        $(SLO)$/admininvokationpage.obj	\
        $(SLO)$/datasourcehandling.obj	\
        $(SLO)$/fieldmappingimpl.obj	\
        $(SLO)$/fieldmappingpage.obj	\
        $(SLO)$/moduleabp.obj		\
        $(SLO)$/tableselectionpage.obj	\
        $(SLO)$/typeselectionpage.obj	\
        $(SLO)$/unodialogabp.obj	\

SLOFILES=	\
        $(EXCEPTIONSFILES)


.IF "$(WITH_MOZILLA)" != "NO"
.IF "$(SYSTEM_MOZILLA)" != "YES"
CDEFS+=-DWITH_MOZILLA
.ENDIF
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=			abspilot.src

RESLIB1NAME=abp
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/abp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(VCLLIB)			\
        $(SALLIB)			\
        $(TOOLSLIB)			\
        $(CPPULIB) 			\
        $(SVTOOLLIB)		\
        $(SVLLIB)			\
        $(SFXLIB)			\
        $(CPPUHELPERLIB)	\
        $(COMPHELPERLIB)	\
        $(TKLIB)			\
        $(UNOTOOLSLIB)		\
        $(SVXCORELIB)		\
        $(SVXLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/abp.component

$(MISC)/abp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        abp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt abp.component
