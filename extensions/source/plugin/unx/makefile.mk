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

PRJNAME=extensions
TARGET=plunx
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  ..$/util$/makefile.pmk

.IF "$(GUIBASE)"=="aqua"
dummy:
    @echo "Nothing to build for GUIBASE aqua."
    
.ELSE

# --- Files --------------------------------------------------------

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
.IF "$(SOLAR_JAVA)" != ""
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
CDEFS+=-DOJI
.ENDIF

.IF "$(WITH_MOZILLA)" != "NO"

.IF "$(DISABLE_XAW)" == "TRUE"
CDEFS+=-DDISABLE_XAW
.ENDIF

SLOFILES=\
    $(SLO)$/nppapi.obj		\
    $(SLO)$/sysplug.obj		\
    $(SLO)$/mediator.obj	\
    $(SLO)$/plugcon.obj		\
    $(SLO)$/unxmgr.obj

OBJFILES=\
    $(OBJ)$/npwrap.obj		\
    $(OBJ)$/npnapi.obj		\
    $(OBJ)$/mediator.obj	\
    $(OBJ)$/plugcon.obj

APP1TARGET=pluginapp.bin
APP1OBJS=$(OBJFILES)
APP1STDLIBS=\
    $(VOSLIB)					\
    $(SALLIB)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="SCO" || "$(OS)"=="HPUX"
APP1STDLIBS+=-lXm -lXt $(X11LINK_DYNAMIC) -ldl
.ELSE
.IF "$(DISABLE_XAW)" != "TRUE"
APP1STDLIBS+=-lXaw 
.ENDIF
.IF "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD"
APP1STDLIBS+= -lXt -lXext -lX11
.ELSE
APP1STDLIBS+= -lXt $(X11LINK_DYNAMIC) -ldl
.ENDIF
.ENDIF

.IF "$(ENABLE_GTK)" == "TRUE"
# libs for gtk plugin
APP1STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
.IF "$(OS)" == "SOLARIS"
# don't ask, it's ugly
DIRECT :=-z nodefs $(DIRECT)
.ENDIF
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""
.ENDIF
    

APP1DEF=	$(MISC)$/$(TARGET).def

.ENDIF # $(WITH_MOZILLA) != "NO"

.ENDIF # $(GUIBASE)==aqua

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

