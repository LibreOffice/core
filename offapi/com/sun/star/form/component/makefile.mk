#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:05:00 $
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
