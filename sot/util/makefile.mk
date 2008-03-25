#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:26:58 $
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

PRJNAME=sot
TARGET=sot

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
.IF "$(GUI)$(COM)"=="WNTGCC"
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(DLLPOSTFIX)_static.a
.ELSE
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
LIB1FILES=	$(SLB)$/base.lib \
            $(SLB)$/sdstor.lib \
            $(SLB)$/unoolestorage.lib

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= $(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=	$(SLB)$/$(TARGET).lib

SHL1STDLIBS=$(TOOLSLIB) $(SALLIB) $(UNOTOOLSLIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(UCBHELPERLIB) $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
         $(PRJ)$/inc$/absdev.hxx	\
         $(PRJ)$/inc$/agg.hxx       \
         $(PRJ)$/inc$/sot$/exchange.hxx  \
         $(PRJ)$/inc$/sot$/factory.hxx   \
         $(PRJ)$/inc$/sot$/object.hxx    \
         $(PRJ)$/inc$/sot$/sotdata.hxx   \
         $(PRJ)$/inc$/sot$/sotref.hxx    \
         $(PRJ)$/inc$/stg.hxx       \
         $(PRJ)$/inc$/sot$/storage.hxx   \
         $(PRJ)$/inc$/storinfo.hxx
DEFLIB1NAME =$(TARGET)
DEF1DES 	=StarObjectsTools

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) sot.flt > $@

