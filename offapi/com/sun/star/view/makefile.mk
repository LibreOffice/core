#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:19:07 $
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

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
