#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: fs $ $Date: 2000-11-02 11:08:58 $
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

PRJ=..

PRJNAME=toolkit
TARGET=tk
VERSION=$(UPD)
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

LDUMP=ldump2.exe

# --- Allgemein ----------------------------------------------------------

.IF "$(depend)" == ""

.IF "$(HEADER)" == ""

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/awt.lib \
            $(SLB)$/controls.lib \
            $(SLB)$/helper.lib
SHL1TARGET= tk$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= itk

SHL1STDLIBS=\
        $(TOOLSLIB)			\
        $(SOTLIB)			\
        $(VOSLIB)			\
        $(COMPHELPERLIB)	\
        $(VCLLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	tk.dxp 

.IF "$(COM)"=="ICC"
SHL1OBJS=	$(SLO)$/unowrap.obj
.ENDIF

.IF "$(COM)"=="WTC"
ALL:		$(LIB1TARGET)						\
            $(MISC)$/$(SHL1TARGET).flt			\
            $(MISC)$/$(SHL1TARGET).def			\
            $(BIN)$/$(SHL1TARGET).dll			\
            ALLTAR
.ELSE
ALL: \
    $(LIB1TARGET)	\
    ALLTAR
.ENDIF


# --- W32 ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

# --- Def-File ---

$(MISC)$/$(SHL1TARGET).def: $(MISC)$/$(SHL1TARGET).flt makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @+-attrib -r defs
.IF "$(COM)"!="WTC"
    @echo LIBRARY	  $(SHL1TARGET) 								 >$@
    @echo DESCRIPTION 'TK'                                        >>$@
.IF "$(COM)"!="ICC"
    @echo DATA		  READWRITE NONSHARED							>>$@
.ENDIF
    @echo EXPORTS													>>$@
.IF "$(COM)"!="BLC"
.IF "$(COM)"!="ICC"
    $(LIBMGR) -EXTRACT:/ /OUT:$(TARGET).exp $(LIB1TARGET)
    @$(LDUMP) -E20 -F$(MISC)$/$(SHL1TARGET).flt $(TARGET).exp		>>$@
    +-del $(TARGET).exp
.ELSE
    @$(LDUMP) -E3 -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)		>>$@
.ENDIF
.ELSE
    @$(LDUMP) -E3 -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)		>>$@
.ENDIF
.ELSE
    @echo option DESCRIPTION 'StarView DLL'                          >$@
    @echo name $(BIN)$/$(SHL1TARGET).dll							>>$@
    @$(LDUMP) -A -E1 -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET) 	>>tmp.def
    @gawk -f s:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF

.ENDIF

# --- OS2 ----------------------------------------------------------------

.IF "$(GUI)" == "OS2"

.IF "$(debug)" != ""
.IF "$(COM)"=="BLC"
LIBFLAGS=$(LIBFLAGS) /P512
.ENDIF
.ENDIF

# --- Def-File ---

$(MISC)$/$(SHL1TARGET).def: $(MISC)$/$(SHL1TARGET).flt makefile.mk
    @echo ------------------------------
    @echo Making: $@
.IF "$(COM)"!="WTC"
    @echo LIBRARY	  $(SHL1TARGET) INITINSTANCE TERMINSTANCE		 >$@
    @echo DESCRIPTION 'VCL'                                        >>$@
    @echo PROTMODE													>>$@
    @echo CODE		  LOADONCALL									>>$@
    @echo DATA		  PRELOAD MULTIPLE NONSHARED					>>$@
    @echo EXPORTS													>>$@
.IF "$(COM)"!="ICC"
    @$(LDUMP) -E1 -A -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET) 	>>$@
.ELSE
    @cppfilt  /b /p /p /n /o $(LIB1TARGET)							>>tmp.cpf
    @$(LDUMP) -A -E1 -F$(MISC)$/$(SHL1TARGET).flt tmp.cpf			>>$@
    @+-del tmp.cpf
.ENDIF
.ELSE
    @echo option DESCRIPTION 'StarView DLL'                          >$@
    @echo name $(BIN)$/$(SHL1TARGET).dll							>>$@
.IF "$(E2P)" != ""
    @echo export e2_dll_begin_tag_.1								>>$@
.ENDIF
    @$(LDUMP) -A -E1 -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET) 	>>tmp.def
    @gawk -f s:\util\exp.awk tmp.def								>>$@
    @+-del tmp.def
.ENDIF

.ENDIF

.IF "$(OS)"=="MACOSX" 
SHL1STDLIBS +=          
.ELIF "$(GUI)"=="UNX"
SHL1STDLIBS +=\
    -lX11 -lXt -lXmu
.ENDIF

# --- Allgemein ----------------------------------------------------------

# --- VCL-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo > $@
#	@echo Impl > $@
    @echo Provider>> $@
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


# --- Targets ------------------------------------------------------------

.ENDIF

.ENDIF
.INCLUDE :	target.mk
