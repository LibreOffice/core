#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 16:21:33 $
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

# 2 == Unicode
MAJOR_VERSION=2

PRJ=..
PRJNAME=package
TARGET=package

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- General ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/zipapi.lib \
    $(SLB)$/zippackage.lib \
    $(SLB)$/manifest.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET=$(TARGET)$(MAJOR_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map

SHL1STDLIBS=\
    $(CPPULIB)		\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)		\
    $(SALLIB)		\
    $(ZLIB3RDLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk

