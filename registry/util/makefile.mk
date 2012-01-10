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



PRJ=..

PRJNAME=registry
TARGET=reg

UNIXVERSIONNAMES=UDK

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------

USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE
LDUMP2=LDUMP3

DOCPPFILES= $(INC)$/registry$/registry.hxx \
            $(INC)$/registry$/registry.h \
            $(INC)$/registry$/regtype.h \
            $(INC)$/registry$/reflread.hxx \
            $(INC)$/registry$/reflwrit.hxx \
            $(INC)$/registry$/refltype.hxx \

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1FILES= 	$(SLB)$/$(TARGET)cpp.lib

SHL1TARGET= $(TARGET)
SHL1IMPLIB= ireg
SHL1STDLIBS= \
    $(SALLIB) \
    $(STORELIB)

SHL1VERSIONMAP=	$(TARGET).map

SHL1LIBS= $(LIB1TARGET)	
SHL1DEPN= $(LIB1TARGET)
SHL1DEF= $(MISC)$/$(SHL1TARGET).def
DEF1NAME= $(SHL1TARGET)
DEF1DES	= RegistryRuntime
SHL1RPATH=URELIB

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

makedocpp: $(DOCPPFILES)
     docpp -H -m -f  -u -d $(OUT)$/doc$/$(PRJNAME) $(DOCPPFILES)

