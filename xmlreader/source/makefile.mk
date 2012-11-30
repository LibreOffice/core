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
PRJNAME = xmlreader
TARGET = xmlreader

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

CDEFS += -DOOO_DLLIMPLEMENTATION_XMLREADER

SLOFILES = \
    $(SLO)/pad.obj \
    $(SLO)/span.obj \
    $(SLO)/xmlreader.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = URELIB
SHL1STDLIBS = \
    $(SALLIB)
SHL1TARGET = xmlreader
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
