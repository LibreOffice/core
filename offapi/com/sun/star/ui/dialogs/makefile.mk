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
# $Revision: 1.15 $
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

PRJNAME=offapi

TARGET=cssuidialogs
PACKAGE=com$/sun$/star$/ui$/dialogs

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    FilterOptionsDialog.idl\
    ControlActions.idl\
    ExecutableDialogException.idl\
    ExecutableDialogResults.idl\
    XExecutableDialog.idl\
    CommonFilePickerElementIds.idl\
    ExtendedFilePickerElementIds.idl\
    FilePicker.idl\
    FilePickerEvent.idl\
    FilePreviewImageFormats.idl\
    FolderPicker.idl\
    ListboxControlActions.idl\
    TemplateDescription.idl\
    XFilePicker.idl\
    XFilePicker2.idl\
    XFilePickerControlAccess.idl	\
    XFilePickerListener.idl\
    XFilePickerNotifier.idl\
    XFilePreview.idl\
    XFilterGroupManager.idl\
    XFilterManager.idl\
    XFolderPicker.idl\
    XControlAccess.idl\
    XControlInformation.idl \
    XAsynchronousExecutableDialog.idl \
    XDialogClosedListener.idl \
    DialogClosedEvent.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
