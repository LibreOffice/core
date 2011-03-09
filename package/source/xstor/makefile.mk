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

PRJNAME=package
TARGET=xstor

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(L10N_framework)"==""

SLOFILES =  \
        $(SLO)$/ohierarchyholder.obj\
        $(SLO)$/ocompinstream.obj\
        $(SLO)$/oseekinstream.obj\
        $(SLO)$/owriteablestream.obj\
        $(SLO)$/xstorage.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/disposelistener.obj\
        $(SLO)$/selfterminatefilestream.obj\
        $(SLO)$/switchpersistencestream.obj\
        $(SLO)$/register.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(SALLIB) 	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

.ENDIF # L10N_framework

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/xstor.component

$(MISC)/xstor.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xstor.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xstor.component
