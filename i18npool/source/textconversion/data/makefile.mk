#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:51:51 $
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
#************************************************************************
PRJ=..$/..$/..

PRJNAME=i18npool
TARGET=textconv_dict
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# Fix heap limit problem on MSC
.IF "$(OS)" == "WNT"
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
    $(BIN)$/genconv_dict $* $< $@

# ugly - is this dependency really required here?
$(foreach,i,$(shell @$(FIND) . -name "*.dic") $(MISC)$/dict_$(i:b).cxx) : $(BIN)$/genconv_dict

