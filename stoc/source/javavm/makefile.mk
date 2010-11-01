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

PRJNAME=stoc
TARGET = javavm.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = jen

.IF "$(SOLAR_JAVA)" == "TRUE"
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# --- Files --------------------------------------------------------

# Kollision zwischen bool.h aus Java und bool.h aus der STL.
# Das Problem tritt fuer alle Plattformen auf, aber anscheinend stolpert nur der
# GCC darueber
.IF "$(COM)" == "GCC"
CDEFS += -D__SGI_STL_BOOL_H
.ENDIF

.INCLUDE :  ..$/cppumaker.mk

SLOFILES= \
        $(SLO)$/javavm.obj		\
        $(SLO)$/jvmargs.obj	\
        $(SLO)$/interact.obj 

SHL1TARGET= $(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1STDLIBS= \
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)	    	\
        $(SALLIB) \
        $(JVMACCESSLIB) \
        $(SALHELPERLIB) \
    $(JVMFWKLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF

SHL1DEPN=		
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=      URELIB

DEF1NAME=		$(SHL1TARGET)

.ELSE		# SOLAR_JAVA
all:
    @echo Nothing to do: SOLAR_JAVA not set
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE :	target.mk

ALLTAR : $(MISC)/javavm.component

$(MISC)/javavm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        javavm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt javavm.component
