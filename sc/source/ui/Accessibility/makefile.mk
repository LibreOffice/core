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

PRJNAME=sc
TARGET=accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  							\
        $(EXCEPTIONSFILES)

EXCEPTIONSFILES=						\
        $(SLO)$/AccessibleCell.obj				\
        $(SLO)$/AccessibleCellBase.obj				\
        $(SLO)$/AccessibleContextBase.obj			\
        $(SLO)$/AccessibleCsvControl.obj			\
        $(SLO)$/AccessibleDataPilotControl.obj			\
        $(SLO)$/AccessibleDocument.obj				\
        $(SLO)$/AccessibleDocumentBase.obj			\
        $(SLO)$/AccessibleDocumentPagePreview.obj		\
        $(SLO)$/AccessibleEditObject.obj			\
        $(SLO)$/AccessibleFilterMenu.obj			\
        $(SLO)$/AccessibleFilterMenuItem.obj			\
        $(SLO)$/AccessibleFilterTopWindow.obj			\
        $(SLO)$/AccessibleGlobal.obj				\
        $(SLO)$/AccessibilityHints.obj				\
        $(SLO)$/AccessiblePageHeader.obj			\
        $(SLO)$/AccessiblePageHeaderArea.obj			\
        $(SLO)$/AccessiblePreviewCell.obj			\
        $(SLO)$/AccessiblePreviewHeaderCell.obj			\
        $(SLO)$/AccessiblePreviewTable.obj			\
        $(SLO)$/AccessibleSpreadsheet.obj			\
        $(SLO)$/AccessibleTableBase.obj				\
        $(SLO)$/AccessibleText.obj				\
        $(SLO)$/DrawModelBroadcaster.obj			\

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


