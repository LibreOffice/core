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

PRJNAME= jvmfwk
TARGET = plugin
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
UNOCOMPONENT1=sunjavaplugin

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.IF "$(SOLAR_JAVA)"!=""

SLOFILES= \
    $(SLO)$/sunversion.obj \
    $(SLO)$/sunjavaplugin.obj \
    $(SLO)$/vendorbase.obj \
    $(SLO)$/util.obj \
    $(SLO)$/sunjre.obj \
    $(SLO)$/gnujre.obj \
    $(SLO)$/vendorlist.obj \
    $(SLO)$/otherjre.obj 

LIB1OBJFILES= $(SLOFILES)

LIB1TARGET=$(SLB)$/$(UNOCOMPONENT1).lib

SHL1TARGET=	$(UNOCOMPONENT1)  


SHL1STDLIBS= \
        $(CPPULIB) \
        $(CPPUHELPER) \
        $(SALLIB) \
        $(SALHELPERLIB)
        

.IF "$(GUI)" == "WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS += uwinapi.lib advapi32.lib
.ELSE
SHL1STDLIBS += -luwinapi -ladvapi32 
.ENDIF # GCC
.ENDIF #WNT

.IF "$(JVM_ONE_PATH_CHECK)" != ""
CFLAGS += -DJVM_ONE_PATH_CHECK=\"$(JVM_ONE_PATH_CHECK)\"
.ENDIF

SHL1VERSIONMAP = sunjavaplugin.map
SHL1DEPN=
SHL1IMPLIB=	i$(UNOCOMPONENT1)
SHL1LIBS=	$(LIB1TARGET) 
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1RPATH=  URELIB

JAVACLASSFILES= \
    $(CLASSDIR)$/JREProperties.class					

JAVAFILES = $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

.ENDIF # SOLAR_JAVA

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"
BOOTSTRAPFILE=$(BIN)$/sunjavaplugin.ini
.ELSE
BOOTSTRAPFILE=$(BIN)$/sunjavapluginrc
.ENDIF


$(BOOTSTRAPFILE): sunjavapluginrc
    -$(COPY) $< $@


ALLTAR: \
    $(BOOTSTRAPFILE)

