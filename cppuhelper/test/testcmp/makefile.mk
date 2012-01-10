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



PRJ               := ..$/..
PRJNAME           := cppuhelper
TARGET            := TestComponent.uno
ENABLE_EXCEPTIONS := TRUE


.INCLUDE: settings.mk


SHL1TARGET     := $(TARGET)
SHL1OBJS       := $(SLO)$/TestComponent.obj
SHL1STDLIBS    := $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) 
SHL1IMPLIB     := i$(SHL1TARGET)
.IF "$(GUI)"=="WNT"
SHL1DEF        := TestComponent.uno.def
.ELSE
DEF1NAME       := $(SHL1TARGET)
.ENDIF


.INCLUDE: target.mk

