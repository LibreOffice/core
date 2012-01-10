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



PRJ=..

PRJNAME=	ucbhelper
TARGET=		ucbhelper

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	..$/version.mk

# --- Files --------------------------------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/client.lib \
            $(SLB)$/provider.lib

SHL1TARGET=	$(TARGET)$(UCBHELPER_MAJOR)$(COMID)
.IF "$(GUI)" == "OS2"
SHL1TARGET=	ucbh$(UCBHELPER_MAJOR)
.ENDIF
SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALHELPERLIB) \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=	$(TARGET)
DEF1DES=	Universal Content Broker - Helpers

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt : ucbhelper.flt
    @echo ------------------------------
    @echo Making: $@
    @$(TYPE) ucbhelper.flt > $@

