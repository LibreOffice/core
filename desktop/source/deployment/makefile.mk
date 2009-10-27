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
# $Revision: 1.18 $
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

PRJ = ..$/..

PRJNAME = desktop
TARGET = deployment
ENABLE_EXCEPTIONS = TRUE
#USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE

.IF "$(GUI)"=="OS2"
TARGET = deploy
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF
 
INCPRE += inc

DLLPRE =

SHL1TARGET = $(TARGET)$(DLLPOSTFIX).uno
SHL1VERSIONMAP = deployment.map

SHL1LIBS = \
    $(SLB)$/deployment_manager.lib \
    $(SLB)$/deployment_migration.lib \
    $(SLB)$/deployment_registry.lib \
    $(SLB)$/deployment_registry_executable.lib \
    $(SLB)$/deployment_registry_component.lib \
    $(SLB)$/deployment_registry_configuration.lib \
    $(SLB)$/deployment_registry_package.lib \
    $(SLB)$/deployment_registry_script.lib \
    $(SLB)$/deployment_registry_sfwk.lib \
    $(SLB)$/deployment_registry_help.lib

SHL1OBJS = \
    $(SLO)$/dp_log.obj \
    $(SLO)$/dp_persmap.obj \
    $(SLO)$/dp_services.obj \
    $(SLO)$/dp_xml.obj

SHL1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB) \
        $(XMLSCRIPTLIB) \
        $(SVLLIB) \
        $(DEPLOYMENTMISCLIB) \
    $(HELPLINKERLIB) \
    $(CONFIGMGRLIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)

SLOFILES = $(LIB1OBJFILES)

RESLIB1NAME = $(TARGET)

RESLIB1SRSFILES = \
        $(SRS)$/deployment_registry_configuration.srs \
        $(SRS)$/deployment_registry_component.srs \
        $(SRS)$/deployment_registry_script.srs \
        $(SRS)$/deployment_registry_sfwk.srs \
        $(SRS)$/deployment_registry_package.srs \
        $(SRS)$/deployment_registry_help.srs \
        $(SRS)$/deployment_registry.srs \
        $(SRS)$/deployment_manager.srs \
    $(SRS)$/deployment_unopkg.srs

.IF "$(GUI)"=="OS2"
RESLIB1SRSFILES += $(SRS)$/deplmisc.srs
.ELSE
RESLIB1SRSFILES += $(SRS)$/deployment_misc.srs
.ENDIF

.INCLUDE : target.mk

