#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2003-04-15 13:45:04 $
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

# tk.dxp should contain all c functions that have to be exported. MT 2001/11/29

PRJ=..

PRJNAME=toolkit
TARGET=tk
VERSION=$(UPD)

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/awt.lib \
            $(SLB)$/controls.lib \
            $(SLB)$/helper.lib
SHL1TARGET= tk$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= itk

SHL1STDLIBS=\
        $(VCLLIB)			\
        $(SOTLIB)			\
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)
DEF1DEPN+=$(DEF1EXPORTFILE)
DEF1DES		=TK
DEFLIB1NAME	=tk

DEF1EXPORTFILE=	tk.dxp 

.IF "$(OS)"=="MACOSX" 

# [ed] 6/16/02 Add in X libraries if we're building X

.IF "$(GUIBASE)"=="unx"
SHL1STDLIBS +=\
    -lX11 -lXt -lXmu
.ENDIF

.ELSE
.IF "$(GUI)"=="UNX"
SHL1STDLIBS +=\
    -lX11
.ENDIF
.ENDIF

RES1FILELIST=$(SRS)$/awt.srs
RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

.INCLUDE :	target.mk

# --- Targets ------------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider> $@
    @echo UnoEditControl>> $@
    @echo UnoControlEditModel>> $@
    @echo UnoFixedTextControl>> $@
    @echo UnoControlFixedTextModel>> $@
    @echo UnoButtonControl>> $@
    @echo UnoControlButtonModel>> $@
    @echo UnoListBoxControl>> $@
    @echo UnoControlListBoxModel>> $@
    @echo VCLXButton>> $@
    @echo VCLXCheckBox>> $@
    @echo VCLXComboBox>> $@
    @echo VCLXFixedText>> $@
    @echo VCLXFontMetric>> $@
    @echo VCLXGraphics>> $@
    @echo VCLXListBox>> $@
    @echo VCLXMenu>> $@
    @echo VCLXMessageBox>> $@
    @echo VCLXRadioButton>> $@
    @echo VCLXScrollBar>> $@
    @echo VCLXVirtualDevice>> $@
    @echo VCLXPopupMenu>> $@
    @echo VCLXMenuBar>> $@
    @echo VCLXToolkit>> $@
    @echo __CT>> $@
