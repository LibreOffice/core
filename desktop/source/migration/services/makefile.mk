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
# $Revision: 1.9 $
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
        $(SLO)$/extensionmigration.obj \
        $(SLO)$/autocorrmigration.obj

SHL1TARGET=$(TARGET)
SHL1VERSIONMAP = migrationoo2.map

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
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

