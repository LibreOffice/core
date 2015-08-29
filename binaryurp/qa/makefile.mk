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



PRJ = ..
PRJNAME = binaryurp
TARGET = qa

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"

@all:
    @echo unit tests are disabled. Nothing to do.

.ELSE


APP1OBJS = $(SLO)/test-cache.obj $(SLO)/main.obj
APP1RPATH = NONE
APP1STDLIBS = $(GTESTLIB) $(SALLIB)
APP1TARGET = test-cache
APP1TEST = enabled

APP2OBJS = \
    $(SLO)/test-unmarshal.obj \
    $(SLO)/binaryany.obj \
    $(SLO)/bridge.obj \
    $(SLO)/bridgefactory.obj \
    $(SLO)/currentcontext.obj \
    $(SLO)/incomingrequest.obj \
    $(SLO)/lessoperators.obj \
    $(SLO)/main.obj \
    $(SLO)/marshal.obj \
    $(SLO)/outgoingrequests.obj \
    $(SLO)/proxy.obj \
    $(SLO)/reader.obj \
    $(SLO)/unmarshal.obj \
    $(SLO)/writer.obj
APP2RPATH = NONE
APP2STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(GTESTLIB) \
    $(SALHELPERLIB) \
    $(SALLIB)
.IF "$(GUI)" != "OS2"
APP2TARGET = test-unmarshal
.ELSE
APP2TARGET = test-u
.ENDIF
APP2TEST = enabled

.INCLUDE: target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
