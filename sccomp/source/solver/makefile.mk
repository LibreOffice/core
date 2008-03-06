#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-03-06 12:48:45 $
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
PRJNAME=sccomp
TARGET=solver

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

SLOFILES=$(SLO)$/solver.obj

SRS1NAME=$(TARGET)
SRC1FILES=solver.src

# --- Library -----------------------------------

SHL1TARGET=     $(TARGET)$(DLLPOSTFIX)

SHL1OBJS=       $(SLOFILES)

SHL1STDLIBS=    $(COMPHELPERLIB)    \
                $(CPPUHELPERLIB)    \
                $(CPPULIB)          \
                $(SALLIB)           \
                $(TOOLSLIB)         \
                $(LPSOLVELIB)

SHL1DEPN=       makefile.mk
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP= exports.map
DEF1NAME=       $(SHL1TARGET)

# --- Resources --------------------------------

RESLIB1LIST=$(SRS)$/solver.srs

RESLIB1NAME=solver
RESLIB1SRSFILES=$(RESLIB1LIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk


