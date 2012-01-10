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
PRJINC=..
PRJNAME=forms
TARGET=component

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/makefile.pmk

# --- Types -------------------------------------

INCPRE+=$(SOLARINCDIR)$/offuh

# --- Files -------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/EventThread.obj
.ENDIF

SLOFILES=	$(SLO)$/Grid.obj \
            $(SLO)$/Columns.obj \
            $(SLO)$/DatabaseForm.obj \
            $(SLO)$/GroupManager.obj \
            $(SLO)$/FormsCollection.obj \
            $(SLO)$/EventThread.obj \
            $(SLO)$/File.obj \
            $(SLO)$/Edit.obj \
            $(SLO)$/EditBase.obj \
            $(SLO)$/Numeric.obj \
            $(SLO)$/Pattern.obj \
            $(SLO)$/Currency.obj \
            $(SLO)$/Date.obj \
            $(SLO)$/Time.obj \
            $(SLO)$/FormattedField.obj \
            $(SLO)$/FormattedFieldWrapper.obj \
            $(SLO)$/ListBox.obj \
            $(SLO)$/ComboBox.obj \
            $(SLO)$/GroupBox.obj \
            $(SLO)$/Hidden.obj \
            $(SLO)$/CheckBox.obj \
            $(SLO)$/RadioButton.obj \
            $(SLO)$/FixedText.obj \
            $(SLO)$/FormComponent.obj \
            $(SLO)$/clickableimage.obj \
            $(SLO)$/Button.obj \
            $(SLO)$/ImageButton.obj \
            $(SLO)$/ImageControl.obj \
            $(SLO)$/errorbroadcaster.obj \
            $(SLO)$/cloneable.obj \
            $(SLO)$/Filter.obj \
            $(SLO)$/entrylisthelper.obj \
            $(SLO)$/scrollbar.obj \
            $(SLO)$/spinbutton.obj \
            $(SLO)$/navigationbar.obj \
            $(SLO)$/formcontrolfont.obj \
            $(SLO)$/refvaluecomponent.obj \
            $(SLO)$/imgprod.obj \
            $(SLO)$/findpos.obj \
            $(SLO)$/propertybaghelper.obj \
            $(SLO)$/cachedrowset.obj \

# --- Targets ----------------------------------

.INCLUDE : target.mk

