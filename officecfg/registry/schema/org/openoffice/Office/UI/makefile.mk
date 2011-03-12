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
TARGET=schema_ooOUI
PACKAGE=org.openoffice.Office.UI

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCSFILES= \
    Controller.xcs \
    Factories.xcs \
    Commands.xcs \
    BasicIDECommands.xcs \
    BibliographyCommands.xcs \
    CalcCommands.xcs \
    ChartCommands.xcs \
    DbuCommands.xcs \
    BaseWindowState.xcs \
    WriterFormWindowState.xcs \
    WriterReportWindowState.xcs \
    DbBrowserWindowState.xcs \
    DbTableDataWindowState.xcs \
    DrawImpressCommands.xcs \
    GenericCommands.xcs \
    MathCommands.xcs \
    StartModuleCommands.xcs \
    WriterCommands.xcs       \
    WindowState.xcs \
    BasicIDEWindowState.xcs \
    BibliographyWindowState.xcs \
    CalcWindowState.xcs \
    ChartWindowState.xcs \
    DbQueryWindowState.xcs \
    DbRelationWindowState.xcs \
    DbTableWindowState.xcs \
    DrawWindowState.xcs \
    ImpressWindowState.xcs \
    MathWindowState.xcs \
    StartModuleWindowState.xcs \
    WriterWindowState.xcs \
    XFormsWindowState.xcs \
    WriterGlobalWindowState.xcs \
    WriterWebWindowState.xcs \
    Effects.xcs \
    Category.xcs \
    GenericCategories.xcs \
    GlobalSettings.xcs \
    WindowContentFactories.xcs

.INCLUDE :  target.mk

