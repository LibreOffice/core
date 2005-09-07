#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:38:27 $
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
#************************************************************************

PRJ=..

PRJNAME=i18npool
TARGET=i18npool.uno

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Allgemein ----------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/defaultnumberingprovider.lib	\
        $(SLB)$/nativenumber.lib		\
        $(SLB)$/registerservices.lib		\
        $(SLB)$/numberformatcode.lib		\
        $(SLB)$/locale.lib			\
        $(SLB)$/indexentry.lib			\
        $(SLB)$/calendar.lib		\
        $(SLB)$/breakiterator.lib \
        $(SLB)$/transliterationImpl.lib \
        $(SLB)$/characterclassification.lib \
        $(SLB)$/collator.lib \
        $(SLB)$/inputchecker.lib \
        $(SLB)$/textconversion.lib

SHL1TARGET= $(TARGET)
# WNT needs implib name even if there is none
SHL1IMPLIB= i$(SHL1TARGET)

SHL1DEPN=	makefile.mk
SHL1VERSIONMAP=$(PRJNAME).map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS= \
        $(I18NUTILLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB)

# --- Targets ------------------------------------------------------------

.INCLUDE :	target.mk
