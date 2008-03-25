#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:26:36 $
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
PRJNAME=unotools
TARGET=utl
TARGETTYPE=CUI
USE_LDUMP2=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/untools.lib
LIB1FILES=\
        $(SLB)$/i18n.lib \
        $(SLB)$/misc.lib \
        $(SLB)$/streaming.lib \
        $(SLB)$/config.lib \
        $(SLB)$/ucbhelp.lib \
        $(SLB)$/procfact.lib \
        $(SLB)$/property.lib \
        $(SLB)$/accessibility.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1IMPLIB=iutl
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(SALHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(I18NISOLANGLIB) \
        $(TOOLSLIB) \
        $(VOSLIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME     =untools
DEF1DES         =unotools

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE		> $@
    @echo _TI					>> $@
    @echo _real					>> $@
    @echo NodeValueAccessor			>> $@
    @echo SubNodeAccess				>> $@
    @echo UpdateFromConfig			>> $@
    @echo UpdateToConfig				>> $@
        @echo _Impl >> $@

