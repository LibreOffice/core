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

PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=bf_go

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_go

# --- Files --------------------------------------------------------

LIB1TARGET=$(SLB)$/bf_go.lib
LIB1FILES=\
   $(SLB)$/goodies_base2d.lib	\
   $(SLB)$/goodies_base3d.lib

SHL1TARGET= bf_go$(DLLPOSTFIX)
SHL1IMPLIB= bf_go
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/bf_go.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=Bf_Goodies
DEFLIB1NAME	=bf_go
#DEF1EXPORTFILE	=bf_goodies.dxp

SHL1STDLIBS= \
            $(SVTOOLLIB)		\
            $(TOOLSLIB)			\
            $(SALLIB)

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

# --- Goodies-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
        @echo _Impl>$@
    @echo WEP>>$@
        @echo m_pLoader>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
        @echo CT>>$@
