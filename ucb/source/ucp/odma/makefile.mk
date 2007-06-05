#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: ihi $ $Date: 2007-06-05 18:08:33 $
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

UCP_VERSION=1
UCP_NAME=odma


PRJ=..$/..$/..
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)
TARGET2=s$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

TARGET2TYPE=CUI
LIBTARGET=NO

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/odma_lib.obj    		\
    $(SLO)$/odma_services.obj    	\
    $(SLO)$/odma_provider.obj    	\
    $(SLO)$/odma_content.obj     	\
    $(SLO)$/odma_resultset.obj		\
    $(SLO)$/odma_datasupplier.obj	\
    $(SLO)$/odma_inputstream.obj	\
    $(SLO)$/odma_contentcaps.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=exports.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)	\
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- SODMA executable -------------------------------------------------
OBJFILES= $(OBJ)$/odma_main.obj 

APP2TARGET= $(TARGET2)
APP2OBJS=   $(OBJFILES)
APP2STDLIBS=$(SALLIB)			\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)

DEF2DES=UCB ODMA URL converter

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

