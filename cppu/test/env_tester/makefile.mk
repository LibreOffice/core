#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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
PRJNAME := cppu
TARGET  := env.tester.bin

ENABLE_EXCEPTIONS := TRUE
NO_BSYMBOLIC      := TRUE


.INCLUDE :  settings.mk
.INCLUDE : ../../source/helper/purpenv/export.mk


.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ObjectFactory_LIB := -lObjectFactory.$(COMID)

.ELSE
ObjectFactory_LIB := $(LIBPRE) iObjectFactory.$(COMID).lib

.ENDIF


APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/env.tester.obj  $(OBJ)$/purpenv.test.obj  $(OBJ)$/register.test.obj  
APP1STDLIBS := $(ObjectFactory_LIB) $(CPPULIB) $(SALLIB) 


SHL1TARGET  	:= purpA_uno_uno
SHL1IMPLIB  	:= i$(SHL1TARGET)
SHL1OBJS    	:= $(SLO)$/TestEnvironment.obj
SHL1STDLIBS 	:= $(CPPULIB) $(SALHELPERLIB) $(SALLIB) 
SHL1DEF         := TestEnvironment.def

SHL2TARGET  	:= purpB_uno_uno
SHL2IMPLIB  	:= i$(SHL2TARGET)
SHL2OBJS    	:= $(SHL1OBJS)
SHL2STDLIBS 	:= $(SHL1STDLIBS)
SHL2DEF         := $(SHL1DEF)


.INCLUDE :  target.mk
