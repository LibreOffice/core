#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:46:59 $
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
TARGET  := cascade_mappping.test.pl

ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
NO_SHL_DESCRIPTION := TRUE


.INCLUDE :  settings.mk


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
