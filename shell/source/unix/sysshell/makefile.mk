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
TARGET=sysshell
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(OS)" != "MACOSX"

SLOFILES=$(SLO)$/recently_used_file.obj \
         $(SLO)$/recently_used_file_handler.obj

SHL1TARGET=recentfile
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

.ELSE
#SHL1IMPLIB=
.ENDIF

SHL1STDLIBS=$(EXPATASCII3RDLIB)\
    $(SALLIB)

SHL1LIBS=$(SLB)$/xmlparser.lib
SHL1OBJS=$(SLOFILES)
SHL1VERSIONMAP=recfile.map

.ELSE

dummy:
    @echo "Nothing to build for MACOSX"

.ENDIF # MACOSX
# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
