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



RTLLIB=irtl.lib

PRJ=..

PRJNAME=vos
TARGET=vos
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  ..$/version.mk

# --- Files --------------------------------------------------------

SHL1TARGET=$(VOS_TARGET)$(VOS_MAJOR)$(COMID)
SHL1IMPLIB=i$(TARGET)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS=$(WSOCK32LIB) $(SALLIB)
.ELSE
SHL1STDLIBS=$(SALLIB)
.ENDIF

SHL1LIBS=    $(SLB)$/cpp$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=    \
    $(SLO)$/object.obj
.ENDIF
.ENDIF

SHL1DEPN=
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =cppvos

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo _alloc >> $@
    @echo alloc >> $@
    @echo _CT >> $@
    @echo _TI2 >> $@
    @echo _TI1 >> $@
    @echo exception::exception >> $@
    @echo @std@ >> $@
    @echo __>>$@

