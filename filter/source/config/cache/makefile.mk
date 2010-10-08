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

PRJ                 = ..$/..$/..

PRJNAME             = filter
TARGET              = filterconfig
ENABLE_EXCEPTIONS   = TRUE
VERSION             = 1
USE_DEFFILE         = TRUE
LIBTARGET           = NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Library -----------------------------------

SHL1TARGET=     $(TARGET)$(VERSION)

SLOFILES=       \
                $(SLO)$/configflush.obj             \
                $(SLO)$/basecontainer.obj           \
                $(SLO)$/cacheitem.obj               \
                $(SLO)$/contenthandlerfactory.obj   \
                $(SLO)$/filtercache.obj             \
                $(SLO)$/filterfactory.obj           \
                $(SLO)$/frameloaderfactory.obj      \
                $(SLO)$/lateinitlistener.obj        \
                $(SLO)$/lateinitthread.obj          \
                $(SLO)$/querytokenizer.obj          \
                $(SLO)$/registration.obj            \
                $(SLO)$/typedetection.obj           \
                $(SLO)$/cacheupdatelistener.obj

SHL1OBJS=       $(SLOFILES)

SHL1STDLIBS=    \
                $(COMPHELPERLIB)    \
                $(CPPUHELPERLIB)    \
                $(CPPULIB)          \
                $(SALLIB)           \
                $(TOOLSLIB)         \
                $(UNOTOOLSLIB)      \
                $(FWELIB)

DEF1NAME=       $(SHL1TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1DEPN=
SHL1IMPLIB=     i$(SHL1TARGET)
SHL1VERSIONMAP= $(SOLARENV)/src/component.map

# --- Targets ----------------------------------

.INCLUDE : target.mk
