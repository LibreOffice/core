#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:53:16 $
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

PRJNAME=xmlscript
TARGET=xcr
NO_BSYMBOLIC=TRUE
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib

LIB1FILES= \
        $(SLB)$/xml_helper.lib		\
        $(SLB)$/xmldlg_imexp.lib	\
        $(SLB)$/xmllib_imexp.lib	\
        $(SLB)$/xmlmod_imexp.lib	\
        $(SLB)$/xmlflat_imexp.lib	\
        $(SLB)$/misc.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1LIBS= \
        $(LIB1TARGET)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)		\
        $(COMPHELPERLIB)		\
        $(CPPULIB) 			\
        $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

# --- Targets -------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk xcr.flt
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) xcr.flt > $@
