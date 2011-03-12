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

PRJNAME=animations
TARGET=animcore
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(L10N_framework)"==""

# --- Files --------------------------------------------------------

SLOFILES = 	$(SLO)$/animcore.obj\
            $(SLO)$/factreg.obj\
            $(SLO)$/targetpropertiescreator.obj

SHL1TARGET= $(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/unloadablecomponent.map

SHL1STDLIBS= \
        $(SALLIB) 	 \
        $(CPPULIB)	 \
        $(CPPUHELPERLIB) 


SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

.ENDIF # L10N_framework

# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk

ALLTAR : $(MISC)/animcore.component

$(MISC)/animcore.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        animcore.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt animcore.component
