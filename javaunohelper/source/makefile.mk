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

PRJNAME=javaunohelper
TARGET=juh
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building javaunohelper because Java is disabled"
.ENDIF

# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc$/comprehensive
INCPRE+=$(OUT)$/inc$/comprehensive
NO_OFFUH=TRUE
CPPUMAKERFLAGS+=-C

UNOTYPES= \
        com.sun.star.beans.NamedValue \
        com.sun.star.container.XHierarchicalNameAccess		\
        com.sun.star.loader.XImplementationLoader		\
        com.sun.star.registry.XRegistryKey			\
        com.sun.star.registry.XSimpleRegistry			\
        com.sun.star.beans.XPropertySet				\
         com.sun.star.lang.DisposedException			\
        com.sun.star.lang.IllegalArgumentException		\
        com.sun.star.lang.XTypeProvider				\
        com.sun.star.lang.XServiceInfo				\
        com.sun.star.lang.XMultiServiceFactory			\
        com.sun.star.lang.XMultiComponentFactory		\
        com.sun.star.lang.XSingleServiceFactory			\
        com.sun.star.lang.XSingleComponentFactory   		\
        com.sun.star.uno.TypeClass				\
        com.sun.star.uno.XWeak					\
        com.sun.star.uno.XAggregation				\
            com.sun.star.uno.XComponentContext          		\
        com.sun.star.lang.XInitialization           		\
        com.sun.star.lang.XComponent

SLOFILES= \
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj					\
        $(SLO)$/preload.obj \
        $(SLO)$/vm.obj

# ------------------------------------------------------------------

.IF "$(GUI)" == "OS2"
SHL1OBJS=\
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj \
        $(SLO)$/vm.obj
.ELSE
LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=\
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj \
        $(SLO)$/vm.obj
SHL1VERSIONMAP = javaunohelper.map
.ENDIF

SHL1TARGET=juhx

SHL1STDLIBS= \
        $(JVMACCESSLIB)		\
        $(SALHELPERLIB)		\
        $(SALLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)

# ------------------------------------------------------------------

.IF "$(GUI)" == "OS2"
SHL2OBJS=\
        $(SLO)$/preload.obj
.ELSE
LIB2TARGET=$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=\
        $(SLO)$/preload.obj
SHL2VERSIONMAP = javaunohelper.map
.ENDIF

SHL2TARGET=juh

SHL2STDLIBS= \
        $(SALLIB)


SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2RPATH=URELIB

DEF2NAME=$(SHL2TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

