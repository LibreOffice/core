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


PRJ=..$/..

PRJNAME=extensions
TARGET=res
LIBTARGET=NO
USE_LDUMP2=TRUE
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/resource.obj \
            $(SLO)$/oooresourceloader.obj \
            $(SLO)$/res_services.obj

LIB1TARGET= 	$(SLB)$/$(TARGET).lib
LIB1OBJFILES=	$(SLOFILES)

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(SALLIB) 	 \
        $(VCLLIB)	\
        $(TOOLSLIB)

SHL1DEPN=
SHL1IMPLIB=		ires
SHL1LIBS=		$(LIB1TARGET)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk


ALLTAR : $(MISC)/res.component

$(MISC)/res.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        res.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt res.component
