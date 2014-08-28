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
TARGET	= spell
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS += -I$(SOLARINCDIR)$/hunspell
.ENDIF

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_HUNSPELL)" == "YES"

CXXFLAGS += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCXX += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCC += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)

EXCEPTIONSFILES=	\
        $(SLO)$/sspellimp.obj

SLOFILES=	\
        $(SLO)$/sreg.obj\
        $(SLO)$/sspellimp.obj

.IF "$(GUI)" == "OS2"
REALNAME:=$(TARGET)
.ELSE
REALNAME:=$(TARGET).uno
.ENDIF
SHL1TARGET= $(REALNAME)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(I18NISOLANGLIB)   \
        $(TOOLSLIB)		\
                $(UNOTOOLSLIB)             \
        $(SVLLIB)		\
        $(SALLIB)		\
        $(LNGLIB) \
        $(ULINGULIB) \
        $(ICUUCLIB) \
        $(HUNSPELLLIB)

# build DLL
SHL1LIBS=       $(SLB)$/$(TARGET).lib $(SLB)$/libulingu.lib
SHL1IMPLIB=		i$(REALNAME)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

# Hunspell is a static library.  Avoid the definition
#     LIBHUNSPELL_DLL_EXPORTED __declspec(dllimport)
#
CDEFS+=-DHUNSPELL_STATIC

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/spell.component

$(MISC)/spell.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        spell.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt spell.component

.ELSE
all:
    @echo "hunspell disabled"
.ENDIF
