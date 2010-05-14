#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
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

PRJNAME=goodies
TARGET=go

#goodies.hid generieren
GEN_HID=TRUE

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/go.lib
LIB1FILES=\
    $(SLB)$/base3d.lib	\
    $(SLB)$/graphic.lib \
    $(SLB)$/unographic.lib	

SHL1TARGET= go$(DLLPOSTFIX)
SHL1IMPLIB= igo

SHL1STDLIBS=\
        $(BASEGFXLIB)		\
        $(VCLLIB)			\
        $(SVLLIB)			\
        $(SVTOOLLIB)		\
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(SALLIB)			\
        $(CPPULIB) 			\
        $(COMPHELPERLIB) 	\
        $(CPPUHELPERLIB)	\
        $(BASEGFXLIB)		\
        $(TKLIB)			

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/go.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)
DEF1DES		=Goodies
DEFLIB1NAME	=go
DEF1EXPORTFILE	=goodies.dxp

# THB: exports list goodies checked for 6.0 Final 6.12.2001
# Note: explicit exports only necessary for VCL graphic filters (see there)

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(depend)"==""

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

.ENDIF
