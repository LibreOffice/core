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



PRJ = ..$/..$/..

PRJNAME	= lingucomponent
TARGET	= lnth
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(MYTHESLIB)"==""
.IF "$(GUI)"=="UNX"
MYTHESLIB=-lmythes
.ENDIF # unx
.IF "$(GUI)"=="OS2"
MYTHESLIB=$(SLB)\libmythes.lib
.ENDIF # os2
.IF "$(GUI)"=="WNT"
MYTHESLIB=libmythes.lib
.ENDIF # wnt
.ENDIF

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_MYTHES)" != "YES"
CXXFLAGS += -I..$/mythes
CFLAGSCXX += -I..$/mythes
CFLAGSCC += -I..$/mythes
.ENDIF
CXXFLAGS += -I$(PRJ)$/source$/lingutil
CFLAGSCXX += -I$(PRJ)$/source$/lingutil
CFLAGSCC += -I$(PRJ)$/source$/lingutil

EXCEPTIONSFILES=	\
        $(SLO)$/nthesimp.obj \
        $(SLO)$/nthesdta.obj

SLOFILES=	\
        $(SLO)$/nthesdta.obj\
        $(SLO)$/ntreg.obj\
        $(SLO)$/nthesimp.obj


REALNAME:=$(TARGET).uno
SHL1TARGET= $(REALNAME)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB)   \
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UNOTOOLSLIB)	\
        $(LNGLIB) \
                $(ULINGULIB) \
        $(MYTHESLIB)

# build DLL
SHL1LIBS=       $(SLB)$/$(TARGET).lib $(SLB)$/libulingu.lib
SHL1IMPLIB=		i$(REALNAME)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/lnth.component

$(MISC)/lnth.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        lnth.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt lnth.component
