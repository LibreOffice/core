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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=forms_component

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
INC+= -I$(PRJ)$/inc$/bf_forms
# --- Types -------------------------------------

INCPRE+=$(SOLARINCDIR)$/offuh

# --- Files -------------------------------------

CXXFILES=	forms_Grid.cxx	\
            forms_Columns.cxx\
            forms_DatabaseForm.cxx \
            forms_GroupManager.cxx \
            forms_FormsCollection.cxx \
            forms_EventThread.cxx \
            forms_File.cxx \
            forms_Edit.cxx \
            forms_EditBase.cxx \
            forms_Numeric.cxx \
            forms_Pattern.cxx \
            forms_Currency.cxx \
            forms_Date.cxx \
            forms_Time.cxx \
            forms_FormattedField.cxx \
            forms_FormattedFieldWrapper.cxx \
            forms_ListBox.cxx \
            forms_ComboBox.cxx \
            forms_GroupBox.cxx \
            forms_Hidden.cxx \
            forms_CheckBox.cxx \
            forms_RadioButton.cxx \
            forms_FixedText.cxx \
            forms_FormComponent.cxx \
            forms_Image.cxx \
            forms_Button.cxx \
            forms_ImageButton.cxx \
            forms_ImageControl.cxx \
            forms_cloneable.cxx \
            forms_imgprod.cxx

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/forms_EventThread.obj
.ENDIF

SLOFILES=	$(SLO)$/forms_Grid.obj \
            $(SLO)$/forms_Columns.obj \
            $(SLO)$/forms_DatabaseForm.obj \
            $(SLO)$/forms_GroupManager.obj \
            $(SLO)$/forms_FormsCollection.obj \
            $(SLO)$/forms_EventThread.obj \
            $(SLO)$/forms_File.obj \
            $(SLO)$/forms_Edit.obj \
            $(SLO)$/forms_EditBase.obj \
            $(SLO)$/forms_Numeric.obj \
            $(SLO)$/forms_Pattern.obj \
            $(SLO)$/forms_Currency.obj \
            $(SLO)$/forms_Date.obj \
            $(SLO)$/forms_Time.obj \
            $(SLO)$/forms_FormattedField.obj \
            $(SLO)$/forms_FormattedFieldWrapper.obj \
            $(SLO)$/forms_ListBox.obj \
            $(SLO)$/forms_ComboBox.obj \
            $(SLO)$/forms_GroupBox.obj \
            $(SLO)$/forms_Hidden.obj \
            $(SLO)$/forms_CheckBox.obj \
            $(SLO)$/forms_RadioButton.obj \
            $(SLO)$/forms_FixedText.obj \
            $(SLO)$/forms_FormComponent.obj \
            $(SLO)$/forms_Image.obj \
            $(SLO)$/forms_Button.obj \
            $(SLO)$/forms_ImageButton.obj \
            $(SLO)$/forms_ImageControl.obj \
            $(SLO)$/forms_errorbroadcaster.obj \
            $(SLO)$/forms_cloneable.obj \
            $(SLO)$/forms_Filter.obj \
            $(SLO)$/forms_imgprod.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

