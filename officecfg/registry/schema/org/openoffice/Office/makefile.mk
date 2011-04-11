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
PRJ=..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=schema_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCSFILES= \
    Accelerators.xcs \
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
    Paths.xcs \
    Histories.xcs \
    OOoImprovement.xcs

.INCLUDE :  target.mk

