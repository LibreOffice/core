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

PRJNAME=cpputools
TARGET=uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS+= -C

UNOTYPES= \
     com.sun.star.uno.TypeClass \
     com.sun.star.uno.XAggregation \
     com.sun.star.uno.XWeak \
     com.sun.star.uno.XComponentContext \
     com.sun.star.lang.XTypeProvider \
    com.sun.star.lang.XMain \
     com.sun.star.lang.XInitialization \
     com.sun.star.lang.XComponent \
     com.sun.star.lang.XSingleServiceFactory \
     com.sun.star.lang.XSingleComponentFactory \
     com.sun.star.lang.XMultiServiceFactory \
     com.sun.star.lang.XMultiComponentFactory \
     com.sun.star.container.XSet \
     com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.loader.XImplementationLoader \
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.registry.XRegistryKey \
    com.sun.star.connection.XAcceptor \
    com.sun.star.connection.XConnection \
    com.sun.star.bridge.XBridgeFactory \
    com.sun.star.bridge.XBridge

# --- Files --------------------------------------------------------

DEPOBJFILES=$(OBJ)$/unoexe.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(DEPOBJFILES)  
APP1RPATH=UREBIN

# Include all relevant (see ure/source/README) dynamic libraries, so that C++
# UNO components running in the uno executable have a defined environment
# (unxlngi6 libstdc++.so.6, and wntmsci10 uwinapi.dll are already
# included via APP1STDLIB, unxlngi6 libgcc_s.so.1 and wntmsci10 msvcr71.dll and
# msvcp71.dll are magic---TODO):
APP1STDLIBS= \
    $(SALLIB)		\
    $(SALHELPERLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)\
    $(LIBXML2LIB)

.INCLUDE :  target.mk

