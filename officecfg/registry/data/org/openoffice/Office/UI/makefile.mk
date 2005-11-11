#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-11-11 11:42:27 $
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
TARGET=data_ooOUI

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

PACKAGE=org.openoffice.Office.UI

XCUFILES= \
    Controller.xcu \
    Factories.xcu \
    BasicIDECommands.xcu \
    BibliographyCommands.xcu \
    CalcCommands.xcu \
    ChartCommands.xcu \
    ChartWindowState.xcu \
    DbuCommands.xcu \
    BaseWindowState.xcu \
    DbQueryWindowState.xcu \
    DbTableWindowState.xcu \
    DbRelationWindowState.xcu \
    DbBrowserWindowState.xcu \
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
    WriterGlobalWindowState.xcu \
    WriterWebWindowState.xcu \
    WriterCommands.xcu\
    Effects.xcu \
    GenericCategories.xcu \
    GlobalSettings.xcu

LOCALIZEDFILES= \
    BasicIDECommands.xcu \
    BibliographyCommands.xcu \
    CalcCommands.xcu \
    ChartCommands.xcu \
    ChartWindowState.xcu \
    DbuCommands.xcu \
    BaseWindowState.xcu \
    DbQueryWindowState.xcu \
    DbTableWindowState.xcu \
    DbRelationWindowState.xcu \
    DbBrowserWindowState.xcu \
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
    WriterGlobalWindowState.xcu \
    WriterWebWindowState.xcu \
    WriterCommands.xcu\
    Effects.xcu \
    GenericCategories.xcu

.INCLUDE :  target.mk

