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
TARGET	= MacOSXSpell
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(GUIBASE)"=="aqua"

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk


# --- Files --------------------------------------------------------


CFLAGSCXX+=$(OBJCXXFLAGS)
CFLAGSCXX+=-I..$/..$/lingutil

EXCEPTIONSFILES=	\
        $(SLO)$/macspellimp.obj


SLOFILES=	\
        $(SLO)$/macreg.obj\
        $(SLO)$/macspellimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(VOSLIB)		\
        $(TOOLSLIB)		\
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)	\
        $(LNGLIB)

SHL1STDLIBS+= -framework Cocoa

# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


.ELSE
dummy:
    @echo " Nothing to build for GUIBASE=$(GUIBASE)"
.ENDIF

ALLTAR : $(MISC)/MacOSXSpell.component

$(MISC)/MacOSXSpell.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt MacOSXSpell.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt MacOSXSpell.component
