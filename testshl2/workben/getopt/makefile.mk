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
PRJINC=

PRJNAME=testshl2
TARGET=test_getopt
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------
#--------------------------------- Objectfiles ---------------------------------
OBJFILES=\
    $(OBJ)$/getopt.obj \
    $(OBJ)$/filehelper.obj \
    $(OBJ)$/test_getopt.obj

#----------------------------- prog with *.lib file -----------------------------

APP1TARGET= $(TARGET)
APP1OBJS=$(OBJFILES)

APP1STDLIBS=$(SALLIB)

APP1DEPN= $(APP1OBJS)
APP1LIBS= 

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
