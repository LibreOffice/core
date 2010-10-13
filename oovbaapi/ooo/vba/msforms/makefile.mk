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

PRJNAME=oovbapi

TARGET=msforms
PACKAGE=ooo$/vba$/msforms

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    MSFormReturnTypes.idl \
    XComboBox.idl \
    XButton.idl \
    XControl.idl \
    XLabel.idl \
    XTextBox.idl \
    XRadioButton.idl \
    XCheckBox.idl \
    XShape.idl \
    XShapes.idl \
    XLineFormat.idl \
    XColorFormat.idl \
    XFillFormat.idl \
    XPictureFormat.idl \
    XShapeRange.idl \
    XTextBoxShape.idl \
    XUserForm.idl \
    XListBox.idl \
    XToggleButton.idl \
    XScrollBar.idl \
    XProgressBar.idl \
    XMultiPage.idl \
    XPages.idl \
    XSpinButton.idl \
    XImage.idl \
    XControls.idl \
    XReturnBoolean.idl \
    XReturnInteger.idl \
    XTextFrame.idl \

# ------------------------------------------------------------------

.INCLUDE :  target.mk
