#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# @@@ UCP Version - Increase, if your UCP libraray becomes incompatible.
UCP_VERSION=1

# @@@ Name for your UCP. Will become part of the library name (See below).
UCP_NAME=myucp

# @@@ Relative path to project root.
PRJ=..$/..

# @@@ Name of the project your UCP code recides it.
PRJNAME=ucbhelper

TARGET=ucp$(UCP_NAME)$(UCP_VERSION).uno

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE :	../../version.mk

# --- General -----------------------------------------------------

# no "lib" prefix
DLLPRE =

# @@@ Adjust template file names. Add own files here.
SLOFILES=\
    $(SLO)$/myucp_services.obj    	\
    $(SLO)$/myucp_provider.obj    	\
    $(SLO)$/myucp_content.obj     	\
    $(SLO)$/myucp_contentcaps.obj   \
    $(SLO)$/myucp_resultset.obj    	\
    $(SLO)$/myucp_datasupplier.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# @@@ Add additional libs here.
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB) \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

