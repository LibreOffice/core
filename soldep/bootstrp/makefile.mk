#*************************************************************************
#
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
#*************************************************************************

PRJ=..

PRJNAME=soldep
TARGET=bootstrpdt
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# fixme, code is not yet 64 bit clean
.IF "$(OS)$(CPU)"=="LINUXX" || ("$(COM)"=="C52" && "$(CPU)"=="U")
all:
    @echo nothing to do
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
                  $(SLO)$/prj.obj

SLOFILES=\
          $(SLO)$/command.obj \
          $(SLO)$/dep.obj \
          $(SLO)$/minormk.obj \
          $(SLO)$/prodmap.obj \
          $(SLO)$/appdef.obj \
           $(SLO)$/hashtbl.obj		\
           $(SLO)$/sstring.obj		\
          $(SLO)$/prj.obj


SHL1TARGET	=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB	=$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS=\
            $(TOOLSLIB) \
            $(VOSLIB)			\
            $(SALLIB)   

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@
