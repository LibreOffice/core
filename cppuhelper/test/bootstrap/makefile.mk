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

PRJ     := ..$/..
PRJNAME := cppuhelper
TARGET  := bootstrap.test


ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
USE_DEFFILE        := TRUE


.INCLUDE :  settings.mk


.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
PURPENVHELPERLIB := -luno_purpenvhelper$(COMID)

.ELSE
PURPENVHELPERLIB := $(LIBPRE) ipurpenvhelper$(UDK_MAJOR)$(COMID).lib

.ENDIF


APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/bootstrap.test.obj
APP1STDLIBS := $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) 

SHL1TARGET      := testenv_uno_uno
SHL1IMPLIB      := i$(SHL1TARGET)
SHL1OBJS        := $(SLO)$/TestEnv.obj
SHL1STDLIBS     := $(PURPENVHELPERLIB) $(SALHELPERLIB) $(SALLIB)
SHL1DEF         := TestEnv.def


.INCLUDE :  target.mk
