#*************************************************************************
#*
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
#************************************************************************
PRJ=..$/..$/..

PRJNAME=i18npool
TARGET=textconv_dict
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# Fix heap limit problem on MSC
.IF "$(COM)" == "MSC"
.IF  "$(COMEX)" != "8"
CDEFS+=-Zm300
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

# collator data library
SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(SHL1TARGET)

SHL1VERSIONMAP=$(SHL1TARGET).map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

DATA_FILES:=$(shell @ls ./*.dic)
SHL1OBJS=$(subst,./,$(SLO)$/ $(DATA_FILES:s/.dic/.obj/))

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

APP1TARGET = genconv_dict
$(MISC)$/%.cxx : %.dic
    $(AUGMENT_LIBRARY_PATH) $(OUT_FOR_BUILD)$/bin$/genconv_dict $* $< $@

# ugly - is this dependency really required here?
$(foreach,i,$(shell @$(FIND) . -name "*.dic") $(MISC)$/dict_$(i:b).cxx) : $(BIN)$/genconv_dict$(EXECPOST)

