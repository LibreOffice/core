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

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = unopkg
.IF "$(GUI)" == "OS2"
TARGETTYPE = CUI
.ELSE
TARGETTYPE = GUI
.ENDIF
ENABLE_EXCEPTIONS = TRUE
LIBTARGET=NO

PRJINC += ..$/..$/deployment ..$/..
.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

.IF "$(LINK_SO)"!=""
APP1TARGET = so$/unopkg
APP1OBJS = $(OBJFILES)
APP1STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
APP1DEPN = $(SHL1TARGETN)
APP1NOSAL = TRUE
APP1RPATH = BRAND
.IF "$(OS)" == "WNT"
APP1ICON = $(SOLARRESDIR)$/icons/so9_main_app.ico
APP1LINKRES = $(MISC)$/$(TARGET)1.res
.ENDIF
.ENDIF			# "$(LINK_SO)"!=""

APP2TARGET = unopkg
APP2OBJS = $(OBJFILES)
APP2STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
APP2DEPN = $(SHL1TARGETN)
APP2NOSAL = TRUE
APP2RPATH = BRAND
.IF "$(OS)" == "WNT"
APP2ICON = $(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP2LINKRES = $(MISC)$/$(TARGET)2.res
.ENDIF

SHL1TARGET = unopkgapp
SHL1OBJS = $(SLOFILES) $(SLO)$/lockfile.obj
SHL1STDLIBS = \
    $(SALLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(VCLLIB) \
    $(DEPLOYMENTMISCLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = \
    $(SLO)$/unopkg_app.obj \
    $(SLO)$/unopkg_cmdenv.obj \
    $(SLO)$/unopkg_misc.obj

OBJFILES = $(OBJ)$/unopkg_main.obj

.INCLUDE : target.mk

.IF "$(APP1TARGETN)" != "" # not set during depend=x
$(APP1TARGETN) : $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

$(MISC)$/binso_created.flg:
    @@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@
    @@-$(MKDIRHIER) $(MISC)$/so && $(TOUCH) $@

