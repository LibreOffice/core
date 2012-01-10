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

PRJNAME=stoc
TARGET = javavm.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = jen

.IF "$(SOLAR_JAVA)" == "TRUE"
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# --- Files --------------------------------------------------------

# Kollision zwischen bool.h aus Java und bool.h aus der STL.
# Das Problem tritt fuer alle Plattformen auf, aber anscheinend stolpert nur der
# GCC darueber
.IF "$(COM)" == "GCC"
CDEFS += -D__SGI_STL_BOOL_H
.ENDIF

.INCLUDE :  ..$/cppumaker.mk

SLOFILES= \
        $(SLO)$/javavm.obj		\
        $(SLO)$/jvmargs.obj	\
        $(SLO)$/interact.obj 

SHL1TARGET= $(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1STDLIBS= \
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)	    	\
        $(SALLIB) \
        $(JVMACCESSLIB) \
        $(SALHELPERLIB) \
    $(JVMFWKLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF

SHL1DEPN=		
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=      URELIB

DEF1NAME=		$(SHL1TARGET)

.ELSE		# SOLAR_JAVA
all:
    @echo Nothing to do: SOLAR_JAVA not set
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE :	target.mk

ALLTAR : $(MISC)/javavm.component

$(MISC)/javavm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        javavm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt javavm.component
