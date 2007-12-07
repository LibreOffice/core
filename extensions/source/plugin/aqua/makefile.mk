#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 11:52:22 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
