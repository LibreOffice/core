#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 16:18:34 $
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
TARGET=data_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
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
    ExtendedColorScheme.xcu

MODULEFILES= \
    Common-writer.xcu   \
    Common-calc.xcu   \
    Common-draw.xcu   \
    Common-impress.xcu   \
    Common-base.xcu   \
    Common-math.xcu   \
    Common-unx.xcu   \
    Common-macosx.xcu   \
    Common-wnt.xcu   \
    Common-dicooo.xcu   \
    Common-UseOOoFileDialogs.xcu \
    Linguistic-ForceDefaultLanguage.xcu \
    Scripting-python.xcu   \
    Common-cjk.xcu   \
    Common-ctl.xcu   \
    Common-korea.xcu   \
    Writer-cjk.xcu \
    Writer-defaultfontarial.xcu \
    Writer-directcursor.xcu \
    Writer-javamail.xcu \
    Embedding-calc.xcu   \
    Embedding-chart.xcu   \
    Embedding-draw.xcu   \
    Embedding-impress.xcu   \
    Embedding-math.xcu   \
    Embedding-base.xcu   \
    Embedding-writer.xcu \
    Embedding-report.xcu


LOCALIZEDFILES= \
    Common.xcu \
    SFX.xcu \
    DataAccess.xcu \
    Embedding.xcu \
    TableWizard.xcu \
    UI.xcu \
    WebWizard.xcu \
    FormWizard.xcu \
    Writer.xcu \
    ExtendedColorScheme.xcu

.INCLUDE :  target.mk

