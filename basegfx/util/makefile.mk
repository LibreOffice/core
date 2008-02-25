#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:43:31 $
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

PRJNAME=basegfx
TARGET=basegfx

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/basegfx.lib
LIB1FILES=\
    $(SLB)$/curve.lib	\
    $(SLB)$/matrix.lib	\
    $(SLB)$/numeric.lib	\
    $(SLB)$/point.lib	\
    $(SLB)$/polygon.lib	\
    $(SLB)$/range.lib	\
    $(SLB)$/tuple.lib	\
    $(SLB)$/tools.lib	\
    $(SLB)$/vector.lib

SHL1TARGET= basegfx$(DLLPOSTFIX)
.IF "$(GUI)" == "OS2"
SHL1TARGET= bgfx
.ENDIF
SHL1IMPLIB= ibasegfx

SHL1STDLIBS=\
        $(SALLIB)		 \
        $(CPPUHELPERLIB) \
        $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/basegfx.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=BaseGFX
DEFLIB1NAME	=basegfx

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @$(TYPE) $(TARGET).flt > $@

