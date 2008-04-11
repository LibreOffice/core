#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.15 $
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
    $(TOOLSLIB) 				\
    $(VOSLIB)					\
    $(SALLIB)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="SCO" || "$(OS)"=="HPUX"
APP1STDLIBS+=-lXm -lXt -lXext -lX11 -ldl
.ELSE
.IF "$(DISABLE_XAW)" != "TRUE"
APP1STDLIBS+=-lXaw 
.ENDIF
.IF "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD"
APP1STDLIBS+= -lXt -lXext -lX11
.ELSE
APP1STDLIBS+= -lXt -lXext -lX11 -ldl
.ENDIF
.ENDIF

APP1DEF=	$(MISC)$/$(TARGET).def

.ENDIF # $(WITH_MOZILLA) != "NO"

.ENDIF # $(GUIBASE)==aqua

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

