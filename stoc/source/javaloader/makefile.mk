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
TARGET = javaloader.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = javaloader

.IF "$(SOLAR_JAVA)" == "TRUE"
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES= 	$(SLO)$/javaloader.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)			\
        $(SALLIB) \
        $(JVMACCESSLIB)

SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1DEPN=		
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS= 		$(SLB)$/$(TARGET).lib 
SHL1DEF= 		$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=      URELIB

DEF1NAME=		$(SHL1TARGET)

.ELSE	# SOLAR_JAVA
all:
    @echo Nothing to do: SOLAR_JAVA not set
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/javaloader.component

$(MISC)/javaloader.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        javaloader.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt javaloader.component
