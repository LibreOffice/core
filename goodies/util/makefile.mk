#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:24:56 $
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
        $(VCLLIB)			\
        $(SVLLIB)			\
        $(SVTOOLLIB)		\
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(VOSLIB)			\
        $(SALLIB)			\
        $(CPPULIB) 			\
        $(COMPHELPERLIB) 	\
        $(UCBHELPERLIB)		\
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
