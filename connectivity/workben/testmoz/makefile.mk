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

PRJNAME=connectivity
TARGET=testmoz
TARGETTYPE=CUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :      settings.mk

# --- Files --------------------------------------------------------
OBJFILES=       $(OBJ)$/main.obj


APPSTDLIBS=$(SALLIB) \
        $(VOSLIB) \
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(DBTOOLSLIB)	\
    $(COMPHELPERLIB)
            

# ... cfgapi ..............................

APP1TARGET= $(TARGET)
APP1OBJS=       $(OBJFILES)
APP1STDLIBS = $(APPSTDLIBS)


    
APP2TARGET = mozThread
APP2OBJS   = $(OBJ)$/initUNO.obj	\
     $(OBJ)$/mozthread.obj
                 
APP2STDLIBS = $(APPSTDLIBS)

# --- Targets ------------------------------------------------------
    
.INCLUDE :      target.mk

