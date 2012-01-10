#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
