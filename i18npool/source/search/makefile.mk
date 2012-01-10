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

PRJNAME=i18npool
TARGET=i18nsearch.uno
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
            $(SLO)$/textsearch.obj

SLOFILES=	\
            $(EXCEPTIONSFILES) \
            $(SLO)$/levdis.obj

# on unxsoli, no optimization for textsearch because of i105945
.IF "$(OS)$(COM)$(CPUNAME)"=="SOLARISC52INTEL"
EXCEPTIONSNOOPTFILES= \
        $(SLO)$/textsearch.obj
.ENDIF

SHL1TARGET= $(TARGET)
SHL1OBJS=	$(SLOFILES)

SHL1STDLIBS= \
                $(CPPULIB) \
                $(CPPUHELPERLIB) \
                $(SALLIB) \
                $(I18NREGEXPLIB) \
                $(ICUINLIB) \
                $(ICUUCLIB)


SHL1DEPN=		makefile.mk
SHL1VERSIONMAP= $(SOLARENV)/src/component.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/i18nsearch.component

$(MISC)/i18nsearch.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        i18nsearch.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt i18nsearch.component
