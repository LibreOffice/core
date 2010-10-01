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
TARGET = javaloader.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = javaloader

.IF "$(SOLAR_JAVA)" == "TRUE"
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES= 	$(SLO)$/javaloader.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)			\
        $(SALLIB) \
        $(JVMACCESSLIB)

SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1DEPN=		
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS= 		$(SLB)$/$(TARGET).lib 
SHL1DEF= 		$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=      URELIB

DEF1NAME=		$(SHL1TARGET)

.ELSE	# SOLAR_JAVA
all:
    @echo Nothing to do: SOLAR_JAVA not set
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/javaloader.component

$(MISC)/javaloader.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        javaloader.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt javaloader.component
