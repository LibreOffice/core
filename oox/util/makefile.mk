#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:08:32 $
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

PRJNAME=oox
TARGET=oox
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/token.lib\
    $(SLB)$/helper.lib\
    $(SLB)$/core.lib\
    $(SLB)$/ppt.lib\
    $(SLB)$/xls.lib\
    $(SLB)$/vml.lib\
    $(SLB)$/drawingml.lib\
    $(SLB)$/diagram.lib\
    $(SLB)$/chart.lib\
    $(SLB)$/shape.lib\
    $(SLB)$/dump.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(VOSLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(RTLLIB)		\
        $(SALLIB)		\
        $(BASEGFXLIB)	\
        $(SAXLIB)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)
DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =$(TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk
