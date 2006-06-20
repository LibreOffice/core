#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 05:56:55 $
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
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(UPD)$(DLLPOSTFIX).a
LIB1FILES=	$(SLB)$/base.lib \
            $(SLB)$/sdstor.lib \
            $(SLB)$/unoolestorage.lib

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= $(TARGET)
SHL1USE_EXPORTS=ordinal
SHL1LIBS=	$(SLB)$/$(TARGET).lib

SHL1STDLIBS=$(TOOLSLIB) $(RTLLIB) $(SALLIB) $(UNOTOOLSLIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(UCBHELPERLIB) $(UNOLIB) $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

.IF "$(linkinc)" != ""
SHL11FILE=	$(MISC)$/base.slo
SHL12FILE=	$(MISC)$/sdstor.slo
.ENDIF

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
         $(PRJ)$/inc$/absdev.hxx	\
         $(PRJ)$/inc$/agg.hxx       \
         $(PRJ)$/inc$/exchange.hxx  \
         $(PRJ)$/inc$/factory.hxx   \
         $(PRJ)$/inc$/object.hxx    \
         $(PRJ)$/inc$/sotdata.hxx   \
         $(PRJ)$/inc$/sotref.hxx    \
         $(PRJ)$/inc$/stg.hxx       \
         $(PRJ)$/inc$/storage.hxx   \
         $(PRJ)$/inc$/storinfo.hxx
DEFLIB1NAME =$(TARGET)
DEF1DES 	=StarObjectsTools

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @+echo ------------------------------
    @+echo Making: $@
    +$(TYPE) sot.flt > $@

