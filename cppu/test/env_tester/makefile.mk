#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:48:25 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
