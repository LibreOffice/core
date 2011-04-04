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
