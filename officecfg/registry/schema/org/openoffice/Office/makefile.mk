#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: hr $ $Date: 2007-11-01 18:03:13 $
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
PRJ=..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=schema_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCSFILES= \
    Addons.xcs \
    Calc.xcs \
    CalcAddIns.xcs \
    Canvas.xcs \
    Chart.xcs \
    Commands.xcs \
    Common.xcs \
    Compatibility.xcs \
    DataAccess.xcs \
    Draw.xcs \
    Events.xcs \
    Embedding.xcs \
    ExtensionManager.xcs \
    Impress.xcs \
    Java.xcs \
    Jobs.xcs \
    Labels.xcs \
    Linguistic.xcs \
    Logging.xcs \
    Math.xcs \
    OptionsDialog.xcs \
    ProtocolHandler.xcs \
    SFX.xcs \
    Substitution.xcs \
    TableWizard.xcs \
    UI.xcs \
    Views.xcs \
    Writer.xcs \
    WriterWeb.xcs \
    Scripting.xcs \
    Security.xcs \
    WebWizard.xcs \
    Recovery.xcs \
    FormWizard.xcs \
    ExtendedColorScheme.xcs \
    TypeDetection.xcs \
    TabBrowse.xcs \
    Paths.xcs	\
    ReportDesign.xcs

.INCLUDE :  target.mk

