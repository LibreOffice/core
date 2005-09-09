#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:43:42 $
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

# @@@ UCP Version - Increase, if your UCP libraray becomes incompatible.
UCP_VERSION=1

# @@@ Name for your UCP. Will become part of the library name (See below).
UCP_NAME=myucp

# @@@ Relative path to project root.
PRJ=..$/..

# @@@ Name of the project your UCP code recides it.
PRJNAME=UCBHELPER

TARGET=ucp$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: svpre.mk
.INCLUDE: settings.mk
.INCLUDE: sv.mk

# --- General -----------------------------------------------------

# @@@ Adjust template file names. Add own files here.
SLOFILES=\
    $(SLO)$/myucp_services.obj    	\
    $(SLO)$/myucp_provider.obj    	\
    $(SLO)$/myucp_content.obj     	\
    $(SLO)$/myucp_contentcaps.obj

# You need these only if you implement a folder content,
# which supports the command "open" returning a result set.
#	$(SLO)$/myucp_resultset.obj
#	$(SLO)$/myucp_datasupplier.obj

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
DEF1DES=UCB Content Provider

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

