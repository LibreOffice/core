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

BFPRJ=..
PRJ=..$/..

PRJNAME=binfilter
TARGET=bf_sb

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=      \
    $(SLB)$/basic_basicmgr.lib \
    $(SLB)$/basic_sbx.lib \
    $(SLB)$/basic_classes.lib \
    $(SLB)$/basic_comp.lib \
    $(SLB)$/basic_runtime.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB=$(TARGET)

SHL1STDLIBS= \
            $(TOOLSLIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(BFSVTOOLLIB) \
            $(SALLIB) \
            $(COMPHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(SOTLIB) \
            $(VCLLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=   \
    $(UWINAPILIB)	\
    $(OLEAUT32LIB)
.ENDIF # WNT

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(SLB)$/$(TARGET).lib

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =   \
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME =$(TARGET)

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk
 
# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo __CT >> $@


