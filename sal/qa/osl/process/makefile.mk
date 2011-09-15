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

PRJNAME=sal
TARGET=qa_osl_process

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

.IF "$(COM)" == "MSC"
    CFLAGS+=/Ob1
.ENDIF

SHL1OBJS=$(SLO)$/osl_Thread.obj
SHL1TARGET=osl_Thread
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB)
SHL1IMPLIB=i$(SHL1TARGET)
DEF1NAME=$(SHL1TARGET)
SHL1USE_EXPORTS = name

SHL2OBJS=$(SLO)$/osl_process.obj
SHL2TARGET=osl_process
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB)
SHL2IMPLIB=i$(SHL2TARGET)
DEF2NAME=$(SHL2TARGET)
SHL2USE_EXPORTS = name

OBJ3FILES=$(OBJ)$/osl_process_child.obj
APP3TARGET=osl_process_child
APP3OBJS=$(OBJ3FILES)
APP3STDLIBS=$(SALLIB)

SHL2DEPN=$(APP3TARGETN) \
         $(BIN)/batch.sh \
         $(BIN)/batch.bat

#------------------------------- All object files -------------------------------
SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(BIN)/batch.sh: batch.sh
    $(COPY) $< $@

$(BIN)/batch.bat: batch.bat
    $(COPY) $< $@

.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
