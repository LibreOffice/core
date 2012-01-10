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

PRJNAME=xmlscript
TARGET=xcr
NO_BSYMBOLIC=TRUE
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib

LIB1FILES= \
        $(SLB)$/xml_helper.lib		\
        $(SLB)$/xmldlg_imexp.lib	\
        $(SLB)$/xmllib_imexp.lib	\
        $(SLB)$/xmlmod_imexp.lib	\
        $(SLB)$/xmlflat_imexp.lib	\
        $(SLB)$/misc.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1LIBS= \
        $(LIB1TARGET)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)		\
        $(CPPULIB) 			\
        $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

# --- Targets -------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk xcr.flt
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) xcr.flt > $@

ALLTAR : $(MISC)/xcr.component

$(MISC)/xcr.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xcr.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xcr.component
