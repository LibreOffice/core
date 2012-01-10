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



PRJ=..$/..$/..$/

PRJNAME=dtrans
TARGET=dndTest
TARGETTYPE=CUI
LIBTARGET=NO

#USE_DEFFILE=	TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

# CFLAGS+=-GR -DUNICODE -D_UNICODE
CFLAGS+= -D_WIN32_DCOM

INCPRE+=	-I$(ATL_INCLUDE)

OBJFILES=	$(OBJ)$/dndTest.obj	\
        $(OBJ)$/atlwindow.obj \
        $(OBJ)$/targetlistener.obj \
        $(OBJ)$/sourcelistener.obj \
        $(OBJ)$/dataobject.obj
        
APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)
APP1OBJS=$(OBJFILES)

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)		\
    $(UWINAPILIB) \
    $(USER32LIB)		\
    $(OLE32LIB)		\
    comsupp.lib		\
    $(OLEAUT32LIB)	\
    $(GDI32LIB)		\
    $(UUIDLIB)

APP1LIBS=	\
            $(SLB)$/dtobjfact.lib	\
            $(SLB)$/dtutils.lib

#			$(SOLARLIBDIR)$/imtaolecb.lib\

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

