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
# $Revision: 1.4 $
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
TARGET  := EnvStack.tester


ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
USE_DEFFILE        := TRUE


.INCLUDE :  settings.mk
.INCLUDE : ../../source/helper/purpenv/export.mk

.IF "$(COM)" == "GCC"
LINKFLAGS += -rdynamic
.ENDIF


ENVINCPRE  := -I$(OUT)$/inc$/$(TARGET)

APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/EnvStack.tester.obj
APP1STDLIBS := $(CPPULIB) $(SALLIB) 


SHL1TARGET  := $(HLD_DLLPRE)A_uno_uno
SHL1IMPLIB  := i$(SHL1TARGET)
SHL1OBJS    := $(SLO)$/ProbeEnv.obj
SHL1STDLIBS := $(purpenv_helper_LIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) 
.IF "$(GUI)"=="WNT"
SHL1STDLIBS += $(BIN)$/EnvStack.tester.lib
.ENDIF
SHL1DEF     := ProbeEnv.def

SHL2TARGET  := $(HLD_DLLPRE)B_uno_uno
SHL2IMPLIB  := i$(SHL2TARGET)
SHL2OBJS    := $(SHL1OBJS)
SHL2STDLIBS := $(SHL1STDLIBS)
SHL2DEF     := $(SHL1DEF)

SHL3TARGET  := $(HLD_DLLPRE)C_uno_uno
SHL3IMPLIB  := i$(SHL3TARGET)
SHL3OBJS    := $(SHL1OBJS)
SHL3STDLIBS := $(SHL1STDLIBS)
SHL3DEF     := $(SHL1DEF)

SHL4TARGET  := $(HLD_DLLPRE)D_uno_uno
SHL4IMPLIB  := i$(SHL4TARGET)
SHL4OBJS    := $(SHL1OBJS)
SHL4STDLIBS := $(SHL1STDLIBS)
SHL4DEF     := $(SHL1DEF)

SHL5TARGET  := $(HLD_DLLPRE)a_uno_uno
SHL5IMPLIB  := i$(SHL5TARGET)
SHL5OBJS    := $(SHL1OBJS)
SHL5STDLIBS := $(SHL1STDLIBS)
SHL5DEF     := $(SHL1DEF)

SHL6TARGET  := $(HLD_DLLPRE)b_uno_uno
SHL6IMPLIB  := i$(SHL6TARGET)
SHL6OBJS    := $(SHL1OBJS)
SHL6STDLIBS := $(SHL1STDLIBS)
SHL6DEF     := $(SHL1DEF)

SHL7TARGET  := $(HLD_DLLPRE)c_uno_uno
SHL7IMPLIB  := i$(SHL7TARGET)
SHL7OBJS    := $(SHL1OBJS)
SHL7STDLIBS := $(SHL1STDLIBS)
SHL7DEF     := $(SHL1DEF)

SHL8TARGET  := $(HLD_DLLPRE)d_uno_uno
SHL8IMPLIB  := i$(SHL8TARGET)
SHL8OBJS    := $(SHL1OBJS)
SHL8STDLIBS := $(SHL1STDLIBS)
SHL8DEF     := $(SHL1DEF)


.INCLUDE :  target.mk


ALLTAR: \
 $(SHL1TARGETN) \
 $(SHL2TARGETN) \
 $(SHL3TARGETN) \
 $(SHL4TARGETN) \
 $(SHL5TARGETN) \
 $(SHL6TARGETN) \
 $(SHL7TARGETN) \
 $(SHL8TARGETN)


$(SHL1TARGETN) : $(APP1TARGETN)
$(SHL2TARGETN) : $(APP1TARGETN)
$(SHL3TARGETN) : $(APP1TARGETN)
$(SHL4TARGETN) : $(APP1TARGETN)
$(SHL5TARGETN) : $(APP1TARGETN)
$(SHL6TARGETN) : $(APP1TARGETN)
$(SHL7TARGETN) : $(APP1TARGETN)
$(SHL8TARGETN) : $(APP1TARGETN)
