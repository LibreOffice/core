#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 12:59:56 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

# @@@ UCP Version - Increase, if your UCP libraray becomes incompatible.
UCP_VERSION=1

# @@@ Name for your UCP. Will become part of the library name (See below).
UCP_NAME=odma

# @@@ Relative path to project root.
PRJ=..$/..$/..

# @@@ Name of the project your UCP code recides it.
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)
TARGET2=s$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

TARGET2TYPE=CUI
LIB2TARGET=NO

# --- Settings ---------------------------------------------------------

.INCLUDE: svpre.mk
.INCLUDE: settings.mk
.INCLUDE: sv.mk

# --- General -----------------------------------------------------

# @@@ Adjust template file names. Add own files here.
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

# @@@ Add additional libs here.
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(VOSLIB)  \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# @@@ A description string for you UCP.
DEF1DES=UCB ODMA Content Provider

# --- SODMA executable -------------------------------------------------
OBJ2FILES= $(OBJ)$/odma_main.obj 

APP2TARGET= $(TARGET2)
APP2OBJS=   $(OBJ2FILES)
APP2STDLIBS=$(SALLIB)			\
            $(VOSLIB)			\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)

APP2STDLIBS+=$(STDLIBCPP)

.IF "$(GUI)"=="WNT"
APP2STDLIBS+=$(LIBCIMT)
.ENDIF

# @@@ A description string for you UCP.
DEF2DES=UCB ODMA URL converter

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

