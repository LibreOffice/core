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

PRJNAME=desktop
TARGET=socomp
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Define time bomb date. Not active for OOo --------------------
# --- Change something in evaluation.cxx!!! (e.g. line 313)
# --- You must use the yyyymmdd format!!! --------------------------
#CDEFS+=-DTIMEBOMB=20050930

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/evaluation.obj \
            $(SLO)$/oemjob.obj \
            $(SLO)$/services.obj 

SHL1DEPN=	makefile.mk
SHL1OBJS=	$(SLOFILES) 


SHL1TARGET=	$(TARGET)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(FWELIB)           \
    $(VCLLIB)           \
    $(SVLLIB)           \
    $(SVTOOLLIB)        \
    $(COMPHELPERLIB)    \
    $(UNOTOOLSLIB)      \
    $(TOOLSLIB)			\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/socomp.component

$(MISC)/socomp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        socomp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt socomp.component
