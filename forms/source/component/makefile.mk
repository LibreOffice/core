#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:51:25 $
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

