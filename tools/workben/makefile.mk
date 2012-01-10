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
PRJNAME = tools
TARGET = workben
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/solar.obj \
    $(OBJ)$/urltest.obj \
    $(OBJ)$/inetmimetest.obj \
    $(OBJ)$/mempooltest.obj

APP1TARGET = solar
APP1OBJS = $(OBJ)$/solar.obj
.IF "$(GUI)" == "UNX" || "$(GUI)" == "OS2"
APP1STDLIBS = $(TOOLSLIB)
.ELSE
APP1LIBS = $(LB)$/itools.lib
.ENDIF

APP2TARGET = urltest
APP2OBJS = $(OBJ)$/urltest.obj
.IF "$(GUI)" == "UNX" || "$(GUI)" == "OS2"
APP2STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ELSE
APP2STDLIBS = $(LB)$/itools.lib $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ENDIF

APP3TARGET = inetmimetest
APP3OBJS = $(OBJ)$/inetmimetest.obj
APP3STDLIBS = $(SALLIB) $(TOOLSLIB)

APP4TARGET = mempooltest
APP4OBJS = $(OBJ)$/mempooltest.obj
APP4STDLIBS = $(TOOLSLIB)
APP4RPATH = UREBIN

.INCLUDE: target.mk
