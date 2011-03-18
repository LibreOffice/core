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
PRJ=..$/..$/..

PRJNAME=oovapi

TARGET=word
PACKAGE=ooo$/vba$/word

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------


IDLFILES= XGlobals.idl\
    XApplication.idl \
    XDocument.idl \
    XWindow.idl \
    XSystem.idl \
    XRange.idl \
    XBookmark.idl \
    XBookmarks.idl \
    XVariable.idl \
    XVariables.idl \
    XView.idl \
    XPane.idl \
    XPanes.idl \
    XOptions.idl \
    XSelection.idl \
     XTemplate.idl \
    XParagraphFormat.idl \
    XAutoTextEntries.idl \
    XAutoTextEntry.idl \
    XParagraphs.idl \
    XParagraph.idl \
    XFind.idl \
    XReplacement.idl \
    XStyle.idl \
    XStyles.idl \
    XFont.idl \
    XTable.idl \
    XTables.idl \
    XField.idl \
    XFields.idl \
    XBorder.idl \
    XBorders.idl \
    XDocuments.idl \
    XHeaderFooter.idl \
    XAddins.idl \
    XAddin.idl \
    XDialogs.idl \
    XDialog.idl \
    XWrapFormat.idl \
    XPageSetup.idl \
    XSection.idl \
    XSections.idl \
    XRow.idl \
    XRows.idl \
    XColumn.idl \
    XColumns.idl \
    XCell.idl \
    XCells.idl \
    XTabStop.idl \
    XTabStops.idl \
    XTableOfContents.idl \
    XTablesOfContents.idl \
    XListFormat.idl \
    XListGalleries.idl \
    XListGallery.idl \
    XListTemplate.idl \
    XListTemplates.idl \
    XListLevel.idl \
    XListLevels.idl \
    XFormField.idl \
    XFormFields.idl \
    XRevision.idl \
    XRevisions.idl \
    XFrame.idl \
    XFrames.idl \
    XCheckBox.idl \
    XHeadersFooters.idl \

# ------------------------------------------------------------------

.INCLUDE :  target.mk

