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

PRJNAME=sax
TARGET=sax.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ELSE
CFLAGS += -DXML_UNICODE
.ENDIF

#-----------------------------------------------------------

SLOFILES =\
        $(SLO)$/xml2utf.obj\
        $(SLO)$/attrlistimpl.obj\
        $(SLO)$/sax_expat.obj \
        $(SLO)$/saxwriter.obj

SHL1TARGET= $(TARGET)
SHL1IMPLIB= i$(TARGET)

SHL1STDLIBS= \
        $(SALLIB)  \
        $(CPPULIB) \
        $(CPPUHELPERLIB)\
        $(EXPAT3RDLIB)

SHL1DEPN=
SHL1VERSIONMAP=	$(SOLARENV)$/src$/component.map
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/sax.component $(MISC)/sax.inbuild.component

$(MISC)/sax.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sax.component

$(MISC)/sax.inbuild.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt sax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_INBUILD_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sax.component
