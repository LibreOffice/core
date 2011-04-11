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

PRJNAME=bridges
TARGET=inter
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/$(TARGET)

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

UNOTYPESTYPES := \
        com.sun.star.lang.IllegalArgumentException \
        com.sun.star.uno.DeploymentException

SLOFILES=$(SLO)$/starter.obj $(SLO)$/thrower.obj

SHL1TARGET=starter
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(SHL1TARGET)
SHL1OBJS=$(SLO)$/starter.obj
DEF1NAME=$(SHL1TARGET)
SHL1STDLIBS+= $(CPPULIB) $(SALLIB)
SHL1VERSIONMAP=$(SHL1TARGET).map

SHL2TARGET=thrower
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2IMPLIB=i$(SHL2TARGET)
SHL2OBJS=$(SLO)$/thrower.obj
DEF2NAME=$(SHL2TARGET)
SHL2STDLIBS+= $(CPPULIB) $(SALLIB)
SHL2VERSIONMAP=$(SHL2TARGET).map

OBJFILES=$(OBJ)$/inter.obj
APP1TARGET=inter
APP1OBJS=$(OBJ)$/inter.obj
APP1STDLIBS+=\
        $(SALLIB)

.INCLUDE :  target.mk

