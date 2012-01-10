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



PRJ     := ..$/..
PRJNAME := cppuhelper
TARGET  := bootstrap.test


ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
USE_DEFFILE        := TRUE


.INCLUDE :  settings.mk


.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
PURPENVHELPERLIB := -luno_purpenvhelper$(COMID)

.ELSE
PURPENVHELPERLIB := $(LIBPRE) ipurpenvhelper$(UDK_MAJOR)$(COMID).lib

.ENDIF


APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/bootstrap.test.obj
APP1STDLIBS := $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) 

SHL1TARGET      := testenv_uno_uno
SHL1IMPLIB      := i$(SHL1TARGET)
SHL1OBJS        := $(SLO)$/TestEnv.obj
SHL1STDLIBS     := $(PURPENVHELPERLIB) $(SALHELPERLIB) $(SALLIB)
SHL1DEF         := TestEnv.def


.INCLUDE :  target.mk
