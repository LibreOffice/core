#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.5 $
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

PRJ=..$/..

PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=sysmgr
ENABLE_EXCEPTIONS=TRUE

# Version
SYSMGR_MAJOR=1

# --- Settings ---

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/makefile.pmk
DLLPRE =

# --- Files ---


SLOFILES=\
    $(SLO)$/systemintegrationmanager.obj \
    $(SLO)$/componentdefn.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib        
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(SYSMGR_MAJOR).uno   
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp    
DEF1DES=Configuration: System Integration Manager

# --- Targets ---

.INCLUDE : target.mk

