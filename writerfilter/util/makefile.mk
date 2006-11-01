#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.2 $
#
#  last change: $Author: hbrinkm $ $Date: 2006-11-01 09:30:33 $
#
#  The Contents of this file are made available subject to
#  the terms of GNU Lesser General Public License Version 2.1.
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
# ***********************************************************************/

PRJ=..
PRJNAME=writerfilter
TARGET=odiapi
TARGET2=writerfilter
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

#SLOFILES=$(SLO)$/PropertiesImpl.obj	

LIB1TARGET=$(SLB)$/godiapi.lib
LIB1FILES=	\
    $(SLB)$/xxml.lib	\
    $(SLB)$/sl.lib	\
    $(SLB)$/props.lib	\
    $(SLB)$/qname.lib#\
#	$(SLB)$/core.lib


SHL1TARGET=$(TARGET)
SHL1LIBS=$(SLB)$/godiapi.lib
SHL1STDLIBS=$(SALLIB) ${LIBXML2LIB} $(CPPULIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(UCBHELPERLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=ordinal
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=godiapi

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=  \
    $(SLB)$/doctok.lib \
    $(SLB)$/dmapper.lib \
    $(SLB)$/filter.lib

SHL2LIBS=$(SLB)$/$(TARGET2).lib


SHL2TARGET=$(TARGET2)$(UPD)$(DLLPOSTFIX)
SHL2STDLIBS=\
    $(I18NISOLANGLIB) \
    $(CPPUHELPERLIB)    \
    $(COMPHELPERLIB)    \
    $(CPPULIB)          \
    $(SALLIB)


SHL2DEPN=
SHL2IMPLIB= i$(SHL2TARGET)
#SHL2LIBS=   $(SLB)$/$(TARGET2).lib
SHL2DEF=    $(MISC)$/$(SHL2TARGET).def
SHL2VERSIONMAP=exports.map

DEF2NAME=$(SHL2TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


