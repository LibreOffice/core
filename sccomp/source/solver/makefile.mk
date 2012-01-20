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
PRJNAME=sccomp
TARGET=solver

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

SLOFILES=$(SLO)$/solver.obj

SRS1NAME=$(TARGET)
SRC1FILES=solver.src

# --- Library -----------------------------------

SHL1TARGET=     $(TARGET)$(DLLPOSTFIX)

SHL1OBJS=       $(SLOFILES)

SHL1STDLIBS=    $(COMPHELPERLIB)    \
                $(CPPUHELPERLIB)    \
                $(CPPULIB)          \
                $(SALLIB)           \
                $(TOOLSLIB)         \
                $(COINMPLIB)

SHL1DEPN=       makefile.mk
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP= $(SOLARENV)/src/component.map
DEF1NAME=       $(SHL1TARGET)

# --- Resources --------------------------------

RESLIB1LIST=$(SRS)$/solver.srs

RESLIB1NAME=solver
RESLIB1SRSFILES=$(RESLIB1LIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk



ALLTAR : $(MISC)/solver.component

$(MISC)/solver.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        solver.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt solver.component

