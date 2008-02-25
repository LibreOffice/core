#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:16:35 $
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

PRJ=..$/..$/..

PRJNAME=goodies
TARGET=epbm
TARGET2=epb
DEPTARGET=vepbm

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)"!="" || "$(EDITDEBUG)"!=""
CDEFS+= -DEDITDEBUG
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=	dlgepbm.src \
            epbmstr.src

SLOFILES =	$(SLO)$/epbm.obj \
            $(SLO)$/dlgepbm.obj

# ==========================================================================

RESLIB1NAME=$(TARGET2)
RESLIB1SRSFILES=$(SRS)$/$(TARGET).srs

SHL1TARGET= 	epb$(DLLPOSTFIX)
SHL1IMPLIB= 	epbm
SHL1STDLIBS=	$(TOOLSLIB) $(VCLLIB) $(CPPULIB)	$(SVTOOLLIB) $(SALLIB)

SHL1LIBS=		$(SLB)$/epbm.lib

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=		$(SLO)$/epbm.obj
.ENDIF
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# ==========================================================================

.INCLUDE :	target.mk
