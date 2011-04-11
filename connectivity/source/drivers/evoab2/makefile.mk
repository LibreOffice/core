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
PRJINC=..$/..
PRJNAME=connectivity
TARGET=evoab2
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

.IF "$(ENABLE_EVOAB2)"!="TRUE"
dummy:
    @echo "Evolution 2.x Addressbook build disabled"
.ELSE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

CFLAGS+=$(GOBJECT_CFLAGS)

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/NDriver.obj \
    $(SLO)$/NTable.obj \
    $(SLO)$/NColumns.obj \
    $(SLO)$/NTables.obj \
    $(SLO)$/NCatalog.obj \
    $(SLO)$/NConnection.obj \
    $(SLO)$/NDatabaseMetaData.obj \
    $(SLO)$/NStatement.obj \
    $(SLO)$/NPreparedStatement.obj \
    $(SLO)$/NServices.obj \
    $(SLO)$/NResultSet.obj  \
    $(SLO)$/NResultSetMetaData.obj \
    $(SLO)$/EApi.obj \
    $(SLO)$/NDebug.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map


# --- Library -----------------------------------

SHL1TARGET=	$(EVOAB_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(SALHELPERLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF
SHL1STDLIBS+=$(GOBJECT_LIBS)


SHL1STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
SHL1SONAME+=-z nodefs
SHL1NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk

ALLTAR : $(MISC)/evoab.component

$(MISC)/evoab.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        evoab.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt evoab.component
