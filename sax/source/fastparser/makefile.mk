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

PRJNAME=sax
TARGET=fastsax.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE = 

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

#-----------------------------------------------------------

SLOFILES =\
        $(SLO)$/facreg.obj\
        $(SLO)$/fastparser.obj\
        $(SLO)$/xml2utf.obj

SHL1TARGET= $(TARGET)
SHL1IMPLIB= i$(TARGET)

SHL1STDLIBS= \
        $(SALLIB)  \
        $(SAXLIB)  \
        $(CPPULIB) \
        $(CPPUHELPERLIB)\
        $(EXPATASCII3RDLIB)

SHL1DEPN=
SHL1VERSIONMAP=	$(SOLARENV)$/src$/component.map
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/fastsax.component

$(MISC)/fastsax.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fastsax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fastsax.component
