#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:18:43 $
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

PRJNAME=registry
TARGET=reg

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.INCLUDE :  ..$/version.mk

# ------------------------------------------------------------------

USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE
LDUMP2=LDUMP3

DOCPPFILES= $(INC)$/registry$/registry.hxx \
            $(INC)$/registry$/registry.h \
            $(INC)$/registry$/regtype.h \
            $(INC)$/registry$/reflread.hxx \
            $(INC)$/registry$/reflwrit.hxx \
            $(INC)$/registry$/refltype.hxx \

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1FILES= 	$(SLB)$/$(TARGET)cpp.lib

SHL1TARGET= $(REGISTRY_TARGET)$(REGISTRY_MAJOR)
SHL1IMPLIB= ireg
SHL1STDLIBS= \
    $(VOSLIB) \
    $(SALLIB) \
    $(STORELIB) \
    $(LIBCIMT)


SHL1LIBS=$(LIB1TARGET)	
SHL1DEPN=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME	=$(SHL1TARGET)

DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt


DEFLIB1NAME =$(TARGET)
DEF1DES 	=RegistryRuntime

.IF "$(COM)"!="ICC" || "$(GUI)"=="OS2"
.IF "$(GUI)"!="MAC"
DEF1EXPORT1		=initRegistry_Api @1000
DEF1EXPORT3		=initRegistryTypeWriter_Api @1001
DEF1EXPORT4		=initRegistryTypeReader_Api @1002
.ELSE
DEF1EXPORT1		=initRegistry_Api
DEF1EXPORT3		=initRegistryTypeWriter_Api
DEF1EXPORT4		=initRegistryTypeReader_Api
.ENDIF
.ELSE
DEF1EXPORT1		=_initRegistry_Api @1000
DEF1EXPORT3		=_initRegistryTypeWriter_Api @1001
DEF1EXPORT4		=_initRegistryTypeReader_Api @1002
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(GUI)"=="WIN"
ALL: $(LIB1TARGET) \
     $(MISC)$/implib.cmd 
     ALLTAR
.ENDIF

.INCLUDE :  target.mk

makedocpp: $(DOCPPFILES)
     + docpp -H -m -f  -u -d $(OUT)$/doc$/$(PRJNAME) $(DOCPPFILES)

# --- SO2-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo _Impl >> $@
    @echo ORegistry >> $@
    @echo ORegKey >> $@
    @echo ORegManager >> $@
    @echo CntStoreFile >> $@
    @echo CntStoreDirectory >> $@
    @echo CntStorePageLockBytes >> $@
    @echo initRegistry >> $@
    @echo _alloc >> $@
    @echo malloc >> $@
    @echo _lower_bound >> $@
    @echo _stl_prime >> $@
    @echo _stl_hash >> $@
    @echo _C >> $@
    @echo _TI2 >> $@
    @echo _TI3 >> $@
    @echo 1Reg >> $@
    @echo 1OGuard >> $@
    @echo _E >> $@
    @echo _H >> $@
    @echo _B >> $@
    @echo _I >> $@
    @echo next >> $@
    @echo HashString >> $@
    @echo distance >> $@
    @echo iterator >> $@
    @echo lower_bound >> $@
    @echo endl >> $@
    @echo flush >> $@
    @echo String >> $@
    @echo Const >> $@
    @echo read >> $@
    @echo write >> $@
    @echo Blop >> $@
    @echo parse >> $@
    @echo Version >> $@
    @echo Entry >> $@
    @echo Type >> $@
    @echo Field >> $@
    @echo magic >> $@
    @echo Pool >> $@
    @echo Method >> $@
    @echo 0CPI >> $@
    @echo exception::exception >> $@






