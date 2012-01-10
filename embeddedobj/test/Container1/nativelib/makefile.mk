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
PRJNAME=embeddedobj
TARGET=nativeview
# LIBTARGET=NO
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VERSIONOBJ=
PACKAGE=embeddedobj$/test

USE_JAVAVER:=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(JAVANUMVER:s/.//)" >= "000100040000"

SLOFILES= $(SLO)$/nativeview.obj

SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(SHL1TARGET)

SHL1STDLIBS= \
    jawt.lib

SHL1OBJS=$(SLOFILES) 
SHL1VERSIONOBJ=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1HEADER=nativeview.h

.ENDIF                  # "$(JAVANUMVER:s/.//)" >= "000100040000"

# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk

.IF "$(JAVANUMVER:s/.//)" >= "000100040000"

$(SLO)$/nativeview.obj : $(SHL1HEADER)

$(SHL1HEADER) :
        javah -classpath $(OUT)$/class -o $(SHL1HEADER) embeddedobj.test

.ENDIF                  # "$(JAVANUMVER:s/.//)" >= "000100040000"


