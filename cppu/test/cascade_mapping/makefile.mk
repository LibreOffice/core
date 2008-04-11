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
# $Revision: 1.5 $
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
TARGET  := cascade_mappping.test.pl

ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE


.INCLUDE :  settings.mk

CFLAGS += -fPIC

VERSIONOBJ := # NO GetVersionInfo symbols :-)
ENVINCPRE  := -I$(OUT)$/inc$/$(TARGET)

HLD_DLLPRE := $(DLLPRE)
DLLPRE     :=


SHL1TARGET  := path.test
SHL1IMPLIB  := i$(SHL1TARGET)
SHL1OBJS    := $(OBJ)$/path.test.obj 
SHL1DEF     := path.test.def
SHL1STDLIBS := $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) 
.IF "$(GUI)"=="WNT"
SHL1STDLIBS += $(BIN)$/mapping.tester.lib
.ENDIF

SHL2TARGET  := $(HLD_DLLPRE)bla_uno_uno
SHL2IMPLIB  := i$(SHL2TARGET)
SHL2OBJS    := $(SLO)$/TestMapping.obj $(SLO)$/TestProxy.obj
SHL2STDLIBS := $(CPPULIB) $(SALHELPERLIB) $(SALLIB) 
SHL2DEF     := TestMapping.def
.IF "$(GUI)"=="WNT"
SHL2STDLIBS += $(BIN)$/mapping.tester.lib
.ENDIF

SHL3TARGET  := $(HLD_DLLPRE)blubb_uno_uno
SHL3IMPLIB  := i$(SHL3TARGET)
SHL3OBJS    := $(SHL2OBJS)
SHL3STDLIBS := $(SHL2STDLIBS)
SHL3DEF     := $(SHL2DEF)

SHL4TARGET  := $(HLD_DLLPRE)bae_uno_uno
SHL4IMPLIB  := i$(SHL4TARGET)
SHL4OBJS    := $(SHL2OBJS)
SHL4STDLIBS := $(SHL2STDLIBS)
SHL4DEF     := $(SHL2DEF)

SHL5TARGET  := $(HLD_DLLPRE)test_uno_uno
SHL5IMPLIB  := i$(SHL5TARGET)
SHL5OBJS    := $(SHL2OBJS)
SHL5STDLIBS := $(SHL2STDLIBS)
SHL5DEF     := $(SHL2DEF)


.INCLUDE :  target.mk


ALLTAR: $(BIN)$/$(TARGET)

$(BIN)$/$(TARGET): cascade_mapping.test.pl
    @+$(COPY) $^ $@
