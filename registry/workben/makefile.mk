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

PRJNAME=registry
TARGET=regtest
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

RGTLIB = rgt.lib
.IF "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
RGTLIB = -lrgt$(DLLPOSTFIX)
.ENDIF


CXXFILES= 	regtest.cxx   	\
            test.cxx		\
            regspeed.cxx


APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regtest.obj 

APP1STDLIBS=\
            $(SALLIB)	\
            $(REGLIB)

APP2TARGET= test
APP2OBJS=   $(OBJ)$/test.obj
APP2RPATH=NONE
.IF "$(GUI)"=="UNX" && "$(OS)"!="MACOSX"

.IF "$(OS)"=="LINUX"
APP2LINKFLAGS=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../lib'\'
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP2LINKFLAGS=-R\''$$ORIGIN/../lib:$$ORIGIN'\'
.ENDIF
    
.ENDIF # "$(OS)"=="UNX"
    
APP2STDLIBS=\
            $(RGTLIB)

APP3TARGET= regspeed
APP3OBJS=   $(OBJ)$/regspeed.obj

APP3STDLIBS=\
            $(SALLIB)	\
            $(REGLIB) 	 

.INCLUDE :  target.mk
