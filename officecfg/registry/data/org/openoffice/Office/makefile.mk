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
TARGET=data_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
    Accelerators.xcu \
    Calc.xcu \
    Canvas.xcu \
    Common.xcu \
    Compatibility.xcu \
    DataAccess.xcu \
    Embedding.xcu \
    ExtensionManager.xcu \
    Impress.xcu	\
    Jobs.xcu \
    Labels.xcu \
    Linguistic.xcu \
    Logging.xcu \
    Math.xcu \
    ProtocolHandler.xcu \
    Security.xcu \
    Scripting.xcu \
    SFX.xcu \
    TableWizard.xcu \
    UI.xcu \
    Views.xcu \
    WebWizard.xcu \
    FormWizard.xcu \
    Writer.xcu \
    Paths.xcu \
    Histories.xcu

MODULEFILES= \
    Accelerators-macosx.xcu \
    Accelerators-unxwnt.xcu \
    Common-brand.xcu \
    Common-writer.xcu   \
    Common-calc.xcu   \
    Common-draw.xcu   \
    Common-impress.xcu   \
    Common-base.xcu   \
    Common-math.xcu   \
    Common-unx.xcu   \
    Common-unixdesktop.xcu \
    Common-gconflockdown.xcu \
    Common-macosx.xcu   \
    Common-wnt.xcu   \
    Common-UseOOoFileDialogs.xcu \
    Linguistic-ForceDefaultLanguage.xcu \
    Scripting-python.xcu   \
    Common-cjk.xcu   \
    Common-ctl.xcu   \
    Common-korea.xcu   \
    Paths-macosx.xcu    \
    Paths-unxwnt.xcu    \
    Paths-unixdesktop.xcu \
    Paths-internallibtextcatdata.xcu \
    Paths-externallibtextcatdata.xcu \
    Writer-cjk.xcu \
    Impress-ogltrans.xcu \
    Embedding-calc.xcu   \
    Embedding-chart.xcu   \
    Embedding-draw.xcu   \
    Embedding-impress.xcu   \
    Embedding-math.xcu   \
    Embedding-base.xcu   \
    Embedding-writer.xcu \
    UI-brand.xcu \
    Recovery-gconflockdown.xcu \
    DataAccess-evoab2.xcu

LOCALIZEDFILES= \
    Accelerators.xcu \
    Common.xcu \
    SFX.xcu \
    DataAccess.xcu \
    TableWizard.xcu \
    UI.xcu \
    Embedding.xcu \
    WebWizard.xcu \
    FormWizard.xcu \
    Writer.xcu


.INCLUDE :  target.mk

