#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
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
APP1STDLIBS= $(SALLIB)

.IF "$(OS)"=="SOLARIS" || "$(OS)"=="SCO"
APP1STDLIBS+=-lXm -lXt $(X11LINK_DYNAMIC) -ldl
.ELSE
.IF "$(DISABLE_XAW)" != "TRUE"
APP1STDLIBS+=-lXaw 
.ENDIF
.IF "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || "$(OS)"=="OPENBSD" || "$(OS)"=="DRAGONFLY"
APP1STDLIBS+= -lXt -lXext -lX11
.ELIF "$(OS)"=="AIX"
APP1STDLIBS+= -lXpm -lXmu -lXt $(X11LINK_DYNAMIC) -ldl
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

