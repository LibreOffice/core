#*************************************************************************
#
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Copyright 2010 Novell, Inc.
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   This file is part of OpenOffice.org.
#
#   OpenOffice.org is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License version 3
#	only, as published by the Free Software Foundation.
#
#   OpenOffice.org is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License version 3 for more details
#	(a copy is included in the LICENSE file that accompanied this code).
#
#   You should have received a copy of the GNU Lesser General Public License
#   version 3 along with OpenOffice.org.  If not, see
#	<http://www.openoffice.org/license.html>
#   for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..
PRJNAME=avmedia
TARGET=avmediagst

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(ENABLE_GSTREAMER)" == "TRUE"

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

PKGCONFIG_MODULES=gstreamer-0.10, gstreamer-plugins-base-0.10
.INCLUDE: pkg_config.mk
PKGCONFIG_LIBS+=-lgstinterfaces-0.10

# --- Files ----------------------------------

.IF "$(GUI)" == "UNX" || "$(GUI)" == "WNT"

.IF "$(GUI)" == "WNT"
CDEFS+= -DWINNT
.ENDIF

SLOFILES= \
        $(SLO)$/gstuno.obj      \
        $(SLO)$/gstmanager.obj  \
        $(SLO)$/gstplayer.obj	\
        $(SLO)$/gstwindow.obj


EXCEPTIONSFILES= \
        $(SLO)$/gstuno.obj      \

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(PKGCONFIG_LIBS) $(TOOLSLIB) $(VCLLIB)
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF # UNX / WNT
.ENDIF # ENABLE_GSTREAMER

.INCLUDE :  	target.mk

.IF "$(ENABLE_GSTREAMER)" == "TRUE"
.IF "$(GUI)" == "UNX" || "$(GUI)" == "WNT"

ALLTAR : $(MISC)/avmediagstreamer.component
$(MISC)/avmediagstreamer.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt avmediagstreamer.component
    $(XSLTPROC) --nonet \
	--stringparam uri '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' \
	-o $@ $(SOLARENV)/bin/createcomponent.xslt avmediagstreamer.component

.ENDIF # UNX / WNT
.ENDIF # ENABLE_GSTREAMER
