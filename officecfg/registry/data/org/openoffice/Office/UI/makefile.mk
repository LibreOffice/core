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
PRJ=..$/..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=data_ooOUI
PACKAGE=org.openoffice.Office.UI

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------


LOCALIZEDFILES= \
    BasicIDECommands.xcu \
    BibliographyCommands.xcu \
    CalcCommands.xcu \
    ChartCommands.xcu \
    ChartWindowState.xcu \
    DbuCommands.xcu \
    BaseWindowState.xcu \
    WriterFormWindowState.xcu \
    WriterReportWindowState.xcu \
    DbQueryWindowState.xcu \
    DbTableWindowState.xcu \
    DbRelationWindowState.xcu \
    DbBrowserWindowState.xcu \
    DbTableDataWindowState.xcu \
    DrawImpressCommands.xcu \
    Effects.xcu \
    GenericCommands.xcu \
    MathCommands.xcu \
    StartModuleCommands.xcu \
    BasicIDEWindowState.xcu \
    CalcWindowState.xcu \
    DrawWindowState.xcu \
    ImpressWindowState.xcu \
    MathWindowState.xcu \
    StartModuleWindowState.xcu \
    WriterWindowState.xcu \
    XFormsWindowState.xcu \
    WriterGlobalWindowState.xcu \
    WriterWebWindowState.xcu \
    WriterCommands.xcu\
    GenericCategories.xcu\

XCUFILES= \
    $(LOCALIZEDFILES) \
    Controller.xcu \
    Factories.xcu \
    GlobalSettings.xcu

.INCLUDE :  target.mk

