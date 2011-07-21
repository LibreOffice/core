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
TARGET=index_data
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

# index data library
SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(SHL1TARGET)

SHL1VERSIONMAP=$(SHL1TARGET).map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

DATA_TXTFILES:=$(shell @ls ./*.txt)
SHL1OBJS=$(subst,./,$(SLO)$/ $(DATA_TXTFILES:s/.txt/.obj/))

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/%.cxx : %.txt
    $(AUGMENT_LIBRARY_PATH) $(OUT_FOR_BUILD)$/bin$/genindex_data $< $@ $*

# ugly - is this dependency really required here?
$(foreach,i,$(shell @$(FIND) . -name "*.txt") $(MISC)$/dict_$(i:b).cxx) : $(BIN)$/genindex_data$(EXECPOST)

