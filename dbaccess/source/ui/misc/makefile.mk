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

PRJ=../../..
PRJINC=$(PRJ)/source
PRJNAME=dbaccess
TARGET=uimisc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)/util/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =    \
        dbumiscres.src    \
        WizardPages.src

# ... object files ............................

SLOFILES=    \
        $(SLO)/asyncmodaldialog.obj         \
        $(SLO)/imageprovider.obj            \
        $(SLO)/singledoccontroller.obj      \
        $(SLO)/dbsubcomponentcontroller.obj \
        $(SLO)/datasourceconnector.obj      \
        $(SLO)/linkeddocuments.obj          \
        $(SLO)/indexcollection.obj          \
        $(SLO)/UITools.obj                  \
        $(SLO)/WCPage.obj                   \
        $(SLO)/WCopyTable.obj               \
        $(SLO)/WTypeSelect.obj              \
        $(SLO)/TokenWriter.obj              \
        $(SLO)/HtmlReader.obj               \
        $(SLO)/RtfReader.obj                \
        $(SLO)/propertysetitem.obj          \
        $(SLO)/databaseobjectview.obj       \
        $(SLO)/DExport.obj                  \
        $(SLO)/uiservices.obj               \
        $(SLO)/RowSetDrop.obj               \
        $(SLO)/TableCopyHelper.obj          \
        $(SLO)/moduledbu.obj                \
        $(SLO)/WColumnSelect.obj            \
        $(SLO)/WExtendPages.obj             \
        $(SLO)/WNameMatch.obj               \
        $(SLO)/ToolBoxHelper.obj            \
        $(SLO)/stringlistitem.obj           \
        $(SLO)/charsets.obj                 \
        $(SLO)/defaultobjectnamecheck.obj   \
        $(SLO)/dsmeta.obj                   \
        $(SLO)/controllerframe.obj          \
        $(SLO)/propertystorage.obj          \
        $(SLO)/dbaundomanager.obj           \

# --- Targets ----------------------------------

.INCLUDE : target.mk

