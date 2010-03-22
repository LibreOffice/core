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
TARGET=onefunc
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------
#--------------------------------- Objectfiles ---------------------------------
OBJFILES=$(OBJ)$/onefuncstarter.obj 
SLOFILES=$(SLO)$/onefunc.obj

# -------------------------------------------------------------------------------

LIB1TARGET=   $(LB)$/$(TARGET)_libfile.lib
LIB1OBJFILES= $(SLOFILES)

.IF "$(GUI)" == "UNX"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF

# ------------------------------------------------------------------
SHL2OBJS = $(SLOFILES)

SHL2TARGET= $(TARGET)_DLL
SHL2STDLIBS+=\
            $(SALLIB)

SHL2DEPN=
SHL2IMPLIB= i$(SHL2TARGET)
SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME    =$(SHL2TARGET)
DEF2EXPORTFILE= export.exp

#----------------------------- prog with *.lib file -----------------------------

APP1TARGET= $(TARGET)starter
APP1OBJS=$(OBJFILES)

APP1STDLIBS=$(SALLIB)

APP1DEPN= $(APP1OBJS) $(LIB1TARGET)
APP1LIBS= $(LIB1TARGET)

#-------------------------- executable with LIBARCHIV --------------------------

APP2TARGET= $(TARGET)starter2
APP2OBJS=   $(OBJFILES)

APP2STDLIBS=$(SALLIB)
.IF "$(GUI)" == "UNX"
APP2STDLIBS+=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
.IF "$(GUI)" == "WNT"
APP2STDLIBS+=$(LIB1TARGET)
.ENDIF

APP2DEPN= $(APP1OBJS) $(LIB1TARGET)
APP2LIBS= 

#----------------------- executable with dynamic library -----------------------

APP3TARGET= $(TARGET)starter3
APP3OBJS=   $(OBJFILES)

APP3STDLIBS=$(SALLIB)

.IF "$(GUI)" == "WNT"
APP3STDLIBS+=i$(SHL2TARGET).lib
.ENDIF
.IF "$(GUI)" == "UNX"
APP3STDLIBS+=-l$(SHL2TARGET)
.ENDIF

APP3DEPN= $(APP1OBJS) $(LIB1TARGET)
APP3LIBS= 
# APP3DEF=

#----------------------- executable without any other files -----------------------

APP4TARGET= $(TARGET)starter4
APP4OBJS=   $(OBJFILES)

APP4STDLIBS=$(SALLIB)

APP4DEPN= $(APP1OBJS)
APP4LIBS= 
# APP3DEF=
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
