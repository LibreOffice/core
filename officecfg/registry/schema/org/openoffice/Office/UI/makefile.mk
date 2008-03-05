#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:39:26 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    DbReportWindowState.xcs \
    ReportCommands.xcs

.INCLUDE :  target.mk

