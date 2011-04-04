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

PRJ=..$/..
PRJNAME=avmediagst
TARGET=avmediagst

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

.IF "$(GUI)" == "UNX"  && "$(GUIBASE)"!="aqua" && "$(ENABLE_GSTREAMER)" != ""

PKGCONFIG_MODULES=gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10
.INCLUDE : pkg_config.mk

SLOFILES= \
        $(SLO)$/gstuno.obj		\
        $(SLO)$/gstmanager.obj		\
        $(SLO)$/gstwindow.obj		\
        $(SLO)$/gstplayer.obj		\
        $(SLO)$/gstframegrabber.obj

EXCEPTIONSFILES= \
        $(SLO)$/gstuno.obj		\
        $(SLO)$/gstplayer.obj		\
        $(SLO)$/gstframegrabber.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(TOOLSLIB) $(VCLLIB)
SHL1STDLIBS+=$(PKGCONFIG_LIBS)
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
# on Solaris checkdll does not work: LD_LIBRARY_PATH breaks the 2 libxml2.so.2
SHL1NOCHECK=t

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

.INCLUDE :  	target.mk

ALLTAR : $(MISC)/avmediagst.component

$(MISC)/avmediagst.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        avmediagst.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediagst.component
