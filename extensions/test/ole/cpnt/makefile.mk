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

PRJNAME=	extensions
TARGET=		oletest
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
#UNOUCRDEP=    $(SOLARBINDIR)$/types.rdb $(BIN)$/oletest.rdb
#UNOUCRRDB=    $(SOLARBINDIR)$/types.rdb $(BIN)$/oletest.rdb

#UNOUCROUT=    $(OUT)$/inc

#UNOTYPES= oletest.XTestSequence \
#    oletest.XTestStruct     \
#    oletest.XTestOther      \
#    oletest.XTestInterfaces \
#    oletest.XSimple         \
#    oletest.XSimple2        \
#    oletest.XSimple3        \
#    oletest.XTestInParameters       \
#    oletest.XIdentity       

INCPRE+= -I$(ATL_INCLUDE)

SLOFILES=	\
        $(SLO)$/cpnt.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)

#.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
#	SHL1STDLIBS+= $(COMPATH)$/atlmfc$/lib$/atls.lib
#.ENDIF

.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
.IF "$(USE_STLP_DEBUG)" != ""
    SHL1STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF


SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


ALLTAR : 	$(MISC)$/$(TARGET).cppumaker.done


.INCLUDE :  target.mk

ALLIDLFILES:=	..$/idl$/oletest.idl

$(BIN)$/oletest.rdb: $(ALLIDLFILES)
    idlc -I$(PRJ) -I$(SOLARIDLDIR)  -O$(BIN) $?
    regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@

$(MISC)$/$(TARGET).cppumaker.done: $(BIN)$/oletest.rdb
    $(CPPUMAKER) -O$(INCCOM) -BUCR $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

