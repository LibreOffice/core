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



PRJ=..

PRJNAME=basic
TARGET=miniapp
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- SBASIC IDE --------------------------------------------------------

APP1TARGET=$(PRJNAME)app
APP1STDLIBS= \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(UNOTOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SJLIB) \
            $(SOTLIB) \
            $(VOSLIB)

#.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
#APP1STDLIBS+=$(CPPULIB)
#.ENDIF
#.IF "$(GUI)"=="UNX"
#APP1STDLIBS+= \
#			$(VOSLIB) \
#			$(SALLIB)
#.ENDIF

.IF "$(GUI)"!="OS2"
APP1LIBS= \
            $(LB)$/basic.lib 
.ENDIF
APP1LIBS+= \
            $(LB)$/app.lib \
            $(LB)$/sample.lib
.IF "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
APP1STDLIBS+=	\
            $(BASICLIB)
.ENDIF


APP1DEPN=	$(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/basic.lib $(LB)$/app.lib $(LB)$/sample.lib

APP1OBJS = $(OBJ)$/ttbasic.obj 

.IF "$(GUI)" != "UNX"
APP1OBJS+=	\
            $(OBJ)$/app.obj \
            $(SLO)$/sbintern.obj
.ENDIF

.INCLUDE :  target.mk

