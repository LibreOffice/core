#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:28:58 $
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
PRJNAME=comphelper
TARGET=comphelper

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/version.mk

# --- Library -----------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/container.lib		\
            $(SLB)$/evtattmgr.lib		\
            $(SLB)$/misc.lib			\
            $(SLB)$/processfactory.lib	\
            $(SLB)$/property.lib		\
            $(SLB)$/streaming.lib		\
            $(SLB)$/compare.lib         \
            $(SLB)$/officeinstdir.lib	\
            $(SLB)$/xml.lib

SHL1TARGET=$(COMPHLP_TARGET)$(COMPHLP_MAJOR)$(COMID)
.IF "$(GUI)" == "OS2"
SHL1TARGET=comph$(COMPHLP_MAJOR)
.ENDIF
SHL1STDLIBS= \
    $(SALLIB) \
    $(SALHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(UCBHELPERLIB) \
    $(VOSLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(COMPHLP_TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
