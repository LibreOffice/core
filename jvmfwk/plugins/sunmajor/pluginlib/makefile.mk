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

PRJNAME= jvmfwk

TARGET = plugin

ENABLE_EXCEPTIONS=TRUE

LIBTARGET=NO

UNOCOMPONENT1=sunjavaplugin

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

#.INCLUDE :  ..$/cppumaker.mk
.IF "$(SOLAR_JAVA)"!=""

SLOFILES= \
    $(SLO)$/sunversion.obj \
    $(SLO)$/sunjavaplugin.obj \
    $(SLO)$/vendorbase.obj \
    $(SLO)$/util.obj \
    $(SLO)$/sunjre.obj \
    $(SLO)$/gnujre.obj \
    $(SLO)$/vendorlist.obj \
    $(SLO)$/otherjre.obj 

LIB1OBJFILES= $(SLOFILES)



LIB1TARGET=$(SLB)$/$(UNOCOMPONENT1).lib

SHL1TARGET=	$(UNOCOMPONENT1)  


SHL1STDLIBS= \
        $(CPPULIB) \
        $(CPPUHELPER) \
        $(SALLIB) \
        $(SALHELPERLIB)
        

.IF "$(GUI)" == "WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS += uwinapi.lib advapi32.lib
.ELSE
SHL1STDLIBS += -luwinapi -ladvapi32 
.ENDIF # GCC
.ENDIF #WNT

SHL1VERSIONMAP = sunjavaplugin.map
SHL1DEPN=
SHL1IMPLIB=	i$(UNOCOMPONENT1)
SHL1LIBS=	$(LIB1TARGET) 
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1RPATH=  URELIB

JAVACLASSFILES= \
    $(CLASSDIR)$/JREProperties.class					

JAVAFILES = $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

.ENDIF # SOLAR_JAVA



# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"
BOOTSTRAPFILE=$(BIN)$/sunjavaplugin.ini
.ELSE
BOOTSTRAPFILE=$(BIN)$/sunjavapluginrc
.ENDIF


$(BOOTSTRAPFILE): sunjavapluginrc
    -$(COPY) $< $@


ALLTAR: \
    $(BOOTSTRAPFILE)

