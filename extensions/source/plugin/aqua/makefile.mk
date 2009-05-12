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
# $Revision: 1.4 $
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
TARGET=plaqua
#ENABLE_EXCEPTIONS=TRUE
#TARGETTYPE=CUI

.INCLUDE :  ..$/util$/makefile.pmk

.IF "$(GUIBASE)"!="aqua" || "$(WITH_MOZILLA)"=="NO"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)."

.ELSE

# --- Files --------------------------------------------------------

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
.IF "$(SOLAR_JAVA)" != ""
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
INCPRE+=-I$(SOLARINCDIR)$/npsdk
CDEFS+=-DOJI
.ENDIF

.IF "$(GUIBASE)"=="aqua"
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)
.ENDIF  # "$(GUIBASE)"=="aqua"

SLOFILES=\
    $(SLO)$/macmgr.obj      \
    $(SLO)$/sysplug.obj

#APP1TARGET=pluginapp.bin
#APP1OBJS=$(OBJFILES)
#APP1STDLIBS=\
#	$(TOOLSLIB) 				\
#	$(VOSLIB)					\
#	$(SALLIB)

#APP1DEF=	$(MISC)$/$(TARGET).def


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.ENDIF # $(GUIBASE)==aqua
