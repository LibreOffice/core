#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


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
        $(VOSLIB)	\
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

