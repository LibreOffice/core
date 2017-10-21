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


PRJ=..$/..$/..

PRJNAME=shell
TARGET=gconfbe

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# no "lib" prefix
DLLPRE =

.IF "$(ENABLE_LOCKDOWN)" == "YES"
CFLAGS+=-DENABLE_LOCKDOWN
.ENDIF

.IF "$(ENABLE_GCONF)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gconf-2.0 gobject-2.0 glib-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS)
.ENDIF

# --- Files ---


SLOFILES=\
    $(SLO)$/gconfaccess.obj \
    $(SLO)$/gconfbackend.obj
        
SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)1.uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)
        
SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF          # "$(ENABLE_GCONF)"!=""

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MISC)/gconfbe1.component

$(MISC)/gconfbe1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        gconfbe1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt gconfbe1.component
