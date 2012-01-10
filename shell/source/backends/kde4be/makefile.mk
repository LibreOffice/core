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


PRJ=..$/..$/..

PRJNAME=shell
TARGET=kde4be

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# For some of the included external KDE headers, GCC complains about shadowed
# symbols in instantiated template code only at the end of a compilation unit,
# so the only solution is to disable that warning here:
.IF "$(COM)" == "GCC"
CFLAGSCXX+=-Wno-shadow
.ENDIF

# no "lib" prefix
DLLPRE =

.IF "$(ENABLE_KDE4)" == "TRUE"

CFLAGS+=$(KDE4_CFLAGS)

# --- Files ---

SLOFILES=\
    $(SLO)$/kde4access.obj \
    $(SLO)$/kde4backend.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)1.uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(KDE4_LIBS) -lkio

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MISC)/kde4be1.component

$(MISC)/kde4be1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        kde4be1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt kde4be1.component
