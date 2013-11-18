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

PRJ=..$/..

PRJNAME=cosv
TARGET=cosv_unittest
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE :  static.mk

.INCLUDE : $(PRJ)$/source$/fullcpp.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/file_ut.obj             \
    $(OBJ)$/simplestring_ut.obj

APP1TARGET=	$(TARGET)
APP1STACK=	1000000
APP1OBJS=   $(OBJ)$/ut_main.obj

APP1LIBS=	$(LB)$/$(TARGET).lib $(LB)$/cosv.lib

APP1DEPN=   $(LB)$/$(TARGET).lib $(LB)$/cosv.lib

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

