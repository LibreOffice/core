#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:33:16 $
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

PRJ=..$/..
PRJNAME=extensions
TARGET=log

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... object files ............................
EXCEPTIONSFILES= \
        $(SLO)$/log_services.obj        \
        $(SLO)$/logger.obj              \
        $(SLO)$/filehandler.obj         \
        $(SLO)$/plaintextformatter.obj  \
        $(SLO)$/consolehandler.obj      \
        $(SLO)$/loghandler.obj          \
        $(SLO)$/loggerconfig.obj        \

SLOFILES= \
        $(EXCEPTIONSFILES) \
        $(SLO)$/log_module.obj          \
        $(SLO)$/logrecord.obj           \

# --- library -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET)_t.lib
LIB1FILES=\
        $(SLB)$/$(TARGET).lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(TARGET).map

# --- Targets ----------------------------------

.INCLUDE : target.mk

