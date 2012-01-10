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


PRJ=..$/..$/..

PRJNAME=oovbapi

TARGET=msforms
PACKAGE=ooo$/vba$/msforms

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    MSFormReturnTypes.idl \
    XCheckBox.idl \
    XComboBox.idl \
    XCommandButton.idl \
    XControl.idl \
    XFrame.idl \
    XLabel.idl \
    XNewFont.idl \
    XTextBox.idl \
    XRadioButton.idl \
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
    XTextFrame.idl \

# ------------------------------------------------------------------

.INCLUDE :  target.mk
