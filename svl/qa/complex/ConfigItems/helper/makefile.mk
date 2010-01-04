#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.4.2 $
#
#   last change: $Author: as $ $Date: 2008/03/19 11:09:25 $
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
PRJ=..$/..$/..$/..

PRJNAME=			svl
TARGET=				ConfigItemTest
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
NO_BSYMBOLIC=		TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- Generate -----------------------------------------------------

INCPOST += $(PRJ)$/source$/inc

# --- light services library ----------------------------------------------------

SHL1TARGET=		svt_$(TARGET)

SHL1OBJS=       \
                $(SLO)$/UserOptTest.obj				\
                $(SLO)$/PrintOptTest.obj			\
                $(SLO)$/AccessibilityOptTest.obj	\
                $(SLO)$/HistoryOptTest.obj			\
                $(SLO)$/ConfigItemTest.obj

SHL1STDLIBS=	\
                $(SVLIB) 						\
                $(SVLLIB) 						    \
                $(UNOTOOLSLIB)						\
                $(COMPHELPERLIB)					\
                $(CPPUHELPERLIB)					\
                $(CPPULIB)							\
                $(SALLIB)

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
#SHL1DEPN=       $(SHL1IMPLIBN) $(SHL1TARGETN)

DEF1NAME=       $(SHL1TARGET)

SHL1VERSIONMAP= exports.map

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk

