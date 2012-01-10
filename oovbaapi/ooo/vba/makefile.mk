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


PRJ=..$/..

PRJNAME=oovbapi

TARGET=vba
PACKAGE=ooo$/vba

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------
.IF "$(L10N_framework)"=""
IDLFILES=\
    XErrObject.idl \
    XCollection.idl\
    XCollectionBase.idl\
    XVBAToOOEventDescGen.idl\
    XPropValue.idl\
    XHelperInterface.idl\
    XAssistant.idl\
    XCommandBarControl.idl\
    XCommandBarControls.idl\
    XCommandBar.idl\
    XCommandBars.idl\
    XCommandBarPopup.idl\
    XCommandBarButton.idl\
    XControlProvider.idl\
    ControlProvider.idl\
    XApplicationBase.idl\
    XWindowBase.idl\
    XDocumentBase.idl\
    XDocumentsBase.idl\
    XGlobalsBase.idl\
    XDocumentProperty.idl\
    XDocumentProperties.idl\
    XExecutableDialog.idl\
    XFontBase.idl\
    XDialogsBase.idl\
    XDialogBase.idl\
    XPageSetupBase.idl

# ------------------------------------------------------------------
.ENDIF
.INCLUDE :  target.mk
