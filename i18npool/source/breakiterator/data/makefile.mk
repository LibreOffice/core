#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.1 $
#*
#*  last change: $Author: bustamam $ $Date: 2002-03-26 06:28:32 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************
PRJ=..$/..$/..

PRJNAME=i18npool
TARGET=dict
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# Fix heap limit problem on MSC
.IF "$(OS)" == "WNT"
CDEFS+=-Zm300
.ENDIF

# --- Files --------------------------------------------------------
# Japanese dictionary
SHL1TARGET=dict_ja
SHL1IMPLIB= 	i$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)
SHL1OBJS= \
    $(SLO)$/dict_ja.obj

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# Thai dictionary
SHL2TARGET=dict_th
SHL2IMPLIB= 	i$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEFLIB2NAME=	$(SHL2TARGET)
SHL2OBJS= \
    $(SLO)$/dict_th.obj

LIB2TARGET=	$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=$(SHL2OBJS)

# Chinese dictionary
SHL3TARGET=dict_zh
SHL3IMPLIB= 	i$(SHL3TARGET)
DEF3DEPN=	$(MISC)$/$(SHL3TARGET).flt
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def
DEF3NAME=	$(SHL3TARGET)
DEFLIB3NAME=	$(SHL3TARGET)
SHL3OBJS= \
    $(SLO)$/dict_zh.obj

LIB3TARGET=	$(SLB)$/$(SHL3TARGET).lib
LIB3OBJFILES=$(SHL3OBJS)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/dict_%.cxx : %.dic $(BIN)$/gendict
    +$(BIN)$/gendict $< $@

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@
