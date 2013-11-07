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
PRJNAME=accessibility
TARGET=accessbridge
LIBTARGET=NO
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VERSIONOBJ=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

SLOFILES= $(SLO)$/WindowsAccessBridgeAdapter.obj

SHL1TARGET=java_uno_accessbridge
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=$(VCLLIB) $(TOOLSLIB) $(JVMACCESSLIB) $(CPPULIB) $(SALLIB)
SHL1OBJS=$(SLOFILES) 
SHL1VERSIONOBJ=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1HEADER=$(OUT)$/inc$/WindowsAccessBridgeAdapter.h

.ENDIF			# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"

$(SLO)$/WindowsAccessBridgeAdapter.obj : $(SHL1HEADER)

$(SHL1HEADER) :
    javah -classpath "$(OUT)$/class;$(OUTDIR)$/bin/ridl.jar;$(OUTDIR)$/bin/unoil.jar;$(CLASSPATH)" -o $(SHL1HEADER) org.openoffice.accessibility.WindowsAccessBridgeAdapter

.ENDIF			# "$(GUI)"=="WNT"
