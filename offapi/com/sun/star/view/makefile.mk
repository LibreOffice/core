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
# $Revision: 1.12 $
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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssview
PACKAGE=com$/sun$/star$/view

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    DocumentZoomType.idl\
    OfficeDocumentView.idl\
    PaperFormat.idl\
    PaperOrientation.idl\
    PrintableState.idl\
    PrintableStateEvent.idl\
    PrinterDescriptor.idl\
    PrintOptions.idl\
    PrintSettings.idl\
    RenderDescriptor.idl\
    RenderOptions.idl\
    ViewSettings.idl\
    XControlAccess.idl\
    XFormLayerAccess.idl\
    XLineCursor.idl\
    XPrintable.idl\
    XPrintableBroadcaster.idl\
    XPrintableListener.idl\
    XPrintPreview.idl\
    XPrintSettingsSupplier.idl\
    XRenderable.idl\
    XScreenCursor.idl\
    XSelectionChangeListener.idl\
    XSelectionSupplier.idl\
    XViewCursor.idl\
    XViewSettingsSupplier.idl\
    PrintJobEvent.idl\
    XPrintJob.idl\
    XPrintJobBroadcaster.idl\
    XPrintJobListener.idl\
    XMultiSelectionSupplier.idl\
    SelectionType.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
