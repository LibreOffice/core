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
PRJNAME=scaddins

TARGET=analysis


ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/types.rdb $(BIN)$/analysisadd.rdb
UNOUCRDEP=$(UNOUCRRDB)

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
NO_OFFUH=TRUE
CPPUMAKERFLAGS*=-C

UNOTYPES=\
    com.sun.star.sheet.addin.XAnalysis \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XSingleComponentFactory \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.sheet.LocalizedName \
    com.sun.star.sheet.XAddIn \
    com.sun.star.sheet.XCompatibilityNames \
    com.sun.star.lang.XServiceName \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.uno.XWeak \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XComponentContext \
    com.sun.star.util.Date \
    com.sun.star.util.XNumberFormatter \
    com.sun.star.util.XNumberFormatTypes \
    com.sun.star.sheet.NoConvergenceException

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/analysis.obj		\
    $(SLO)$/financial.obj		\
    $(SLO)$/analysishelper.obj	\
    $(SLO)$/bessel.obj

# SCO: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="SCO"
SLOFILES+=$(SLO)$/staticmb.obj
.ENDIF

ALLIDLFILES=\
    analysisadd.idl

SRS1NAME=$(TARGET)
SRC1FILES =  \
        analysis.src		\
        analysis_funcnames.src	\
        analysis_deffuncnames.src

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(TOOLSLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1DEPN=makefile.mk

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/analysis.srs

RESLIB1NAME=analysis
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(BIN)$/analysisadd.rdb: $(ALLIDLFILES)
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@



ALLTAR : $(MISC)/analysis.component

$(MISC)/analysis.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        analysis.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt analysis.component
