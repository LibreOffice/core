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

PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=cssfcomp
PACKAGE=com$/sun$/star$/form$/component

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    CheckBox.idl\
    ComboBox.idl\
    CommandButton.idl\
    CurrencyField.idl\
    DatabaseCheckBox.idl\
    DatabaseComboBox.idl\
    DatabaseCurrencyField.idl\
    DatabaseDateField.idl\
    DatabaseImageControl.idl\
    DatabaseListBox.idl\
    DatabaseNumericField.idl\
    DatabasePatternField.idl\
    DatabaseRadioButton.idl\
    DatabaseTextField.idl\
    DatabaseFormattedField.idl\
    DatabaseTimeField.idl\
    DataForm.idl\
    DateField.idl\
    FileControl.idl\
    FixedText.idl\
    Form.idl\
    GridControl.idl\
    GroupBox.idl\
    HiddenControl.idl\
    HTMLForm.idl\
    ImageButton.idl\
    ListBox.idl\
    NumericField.idl\
    PatternField.idl\
    RadioButton.idl\
    TextField.idl\
    FormattedField.idl\
    TimeField.idl\
    NavigationToolBar.idl\
    ScrollBar.idl\
    SpinButton.idl\
    SubmitButton.idl\
    RichTextControl.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
