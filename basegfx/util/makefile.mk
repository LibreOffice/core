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

PRJNAME=basegfx
TARGET=basegfx

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/basegfx.lib
LIB1FILES=\
    $(SLB)$/curve.lib	\
    $(SLB)$/matrix.lib	\
    $(SLB)$/numeric.lib	\
    $(SLB)$/point.lib	\
    $(SLB)$/polygon.lib	\
    $(SLB)$/range.lib	\
    $(SLB)$/tuple.lib	\
    $(SLB)$/tools.lib	\
    $(SLB)$/vector.lib	\
    $(SLB)$/color.lib	\
    $(SLB)$/pixel.lib	\
    $(SLB)$/raster.lib

SHL1TARGET= basegfx$(DLLPOSTFIX)
.IF "$(GUI)" == "OS2"
SHL1TARGET= bgfx
.ENDIF
SHL1IMPLIB= ibasegfx

SHL1STDLIBS=\
        $(SALLIB)		 \
        $(CPPUHELPERLIB) \
        $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/basegfx.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=BaseGFX
DEFLIB1NAME	=basegfx

LIB2TARGET=$(LB)$/$(TARGET)_s.lib
LIB2ARCHIV=$(LB)$/lib$(TARGET)_s.a
LIB2FILES=$(LIB1FILES)

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @$(TYPE) $(TARGET).flt > $@

