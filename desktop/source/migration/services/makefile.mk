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

PRJNAME=desktop
TARGET = migrationoo2.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = migrationoo2
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE : ..$/..$/deployment/inc/dp_misc.mk
.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  cppumaker.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

SLOFILES= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj \
        $(SLO)$/oo3extensionmigration.obj \
        $(SLO)$/cexportsoo3.obj

SHL1OBJS= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj

SHL1TARGET=$(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1STDLIBS= \
    $(DEPLOYMENTMISCLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(SALLIB) \
    $(UCBHELPERLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(JVMFWKLIB) \
    $(XMLSCRIPTLIB) \
    $(BERKELEYLIB)

SHL1DEPN=
SHL1IMPLIB=imigrationoo2
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

COMP2TYPELIST = migrationoo3
SHL2TARGET=migrationoo3.uno
SHL2VERSIONMAP = $(SOLARENV)/src/component.map

SHL2OBJS= \
        $(SLO)$/cexportsoo3.obj \
        $(SLO)$/oo3extensionmigration.obj

SHL2STDLIBS= \
    $(DEPLOYMENTMISCLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(SALLIB) \
    $(UCBHELPERLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(JVMFWKLIB) \
    $(XMLSCRIPTLIB) \
    $(BERKELEYLIB)

SHL2DEPN=
SHL2IMPLIB=imigrationoo3
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/migrationoo3.component

$(MISC)/migrationoo3.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo3.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo3.component

ALLTAR : $(MISC)/migrationoo2.component

$(MISC)/migrationoo2.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
