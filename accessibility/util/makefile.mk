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
PRJNAME=accessibility
TARGET=acc
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

#.INCLUDE :	svpre.mk
#.INCLUDE :	settings.mk
#.INCLUDE :	sv.mk

LDUMP=ldump2.exe

# --- Library -----------------------------------
# --- acc ---------------------------------------
LIB1TARGET=$(SLB)$/$(PRJNAME).lib
LIB1FILES=\
        $(SLB)$/standard.lib \
        $(SLB)$/extended.lib \
        $(SLB)$/helper.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(VCLLIB)           \
        $(COMPHELPERLIB)    \
        $(SOTLIB)           \
        $(CPPULIB)          \
        $(CPPUHELPERLIB)    \
        $(UNOTOOLSLIB)      \
        $(TKLIB)            \
        $(TOOLSLIB)         \
        $(SVTOOLLIB)        \
        $(SVLLIB)           \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEPN=$(LIB1TARGET)	\
        makefile.mk


SHL1VERSIONMAP= $(TARGET).map 
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

# === .res file ==========================================================

RES1FILELIST=\
            $(SRS)$/helper.srs

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

