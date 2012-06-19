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
PRJNAME=lingucomponent

.IF "$(GUI)"=="OS2"
TARGET=guesslan
.ELSE
TARGET=guesslang
.ENDIF

ENABLE_EXCEPTIONS=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
LIBTEXTCATLIB=-ltextcat
.ELSE               # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
LIBTEXTCATLIB=ilibtextcat.lib
.ENDIF  # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

SLOFILES = \
        $(SLO)$/altstrfunc.obj \
        $(SLO)$/guess.obj \
        $(SLO)$/guesslang.obj \
        $(SLO)$/simpleguesser.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)        \
        $(CPPULIB)              \
        $(LIBTEXTCATLIB)        \
        $(SALLIB)               \
        $(SVLLIB)               \
        $(TOOLSLIB)             \
        $(UNOTOOLSLIB)

# build DLL
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
.IF "$(OS)"!="MACOSX"
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
.ENDIF

# build DEF file
DEF1NAME    =$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/guesslang.component

$(MISC)/guesslang.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        guesslang.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt guesslang.component
