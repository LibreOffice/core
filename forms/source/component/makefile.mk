#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:29:06 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

CXXFILES=	Grid.cxx	\
            Columns.cxx\
            DatabaseForm.cxx \
            GroupManager.cxx \
            FormsCollection.cxx \
            EventThread.cxx \
            File.cxx \
            Edit.cxx \
            EditBase.cxx \
            Numeric.cxx \
            Pattern.cxx \
            Currency.cxx \
            Date.cxx \
            Time.cxx \
            FormattedField.cxx \
            FormattedFieldWrapper.cxx \
            ListBox.cxx \
            ComboBox.cxx \
            GroupBox.cxx \
            Hidden.cxx \
            CheckBox.cxx \
            RadioButton.cxx \
            FixedText.cxx \
            FormComponent.cxx \
            Image.cxx \
            Button.cxx \
            ImageButton.cxx \
            ImageControl.cxx \

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
            $(SLO)$/Image.obj \
            $(SLO)$/Button.obj \
            $(SLO)$/ImageButton.obj \
            $(SLO)$/ImageControl.obj \

# --- Targets ----------------------------------

.INCLUDE : target.mk

