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
# $Revision: 1.16 $
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
PRJINC=..
PRJNAME=forms
TARGET=component

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

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

