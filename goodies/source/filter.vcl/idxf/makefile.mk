#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:21:40 $
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
TARGET=idxf
DEPTARGET=vidxf

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)"!="" || "$(EDITDEBUG)"!=""
CDEFS+= -DEDITDEBUG
.ENDIF

EXCEPTIONSFILES=\
            $(SLO)$/dxfentrd.obj \
            $(SLO)$/dxf2mtf.obj

SLOFILES =  $(SLO)$/dxfgrprd.obj \
            $(SLO)$/dxfvec.obj   \
            $(SLO)$/dxfentrd.obj \
            $(SLO)$/dxfblkrd.obj \
            $(SLO)$/dxftblrd.obj \
            $(SLO)$/dxfreprd.obj \
            $(SLO)$/dxf2mtf.obj  \
            $(SLO)$/idxf.obj

# ==========================================================================

SHL1TARGET=     idx$(DLLPOSTFIX)
SHL1IMPLIB=     idxf
SHL1STDLIBS=    $(VCLLIB) $(TOOLSLIB) $(SALLIB)

SHL1LIBS=       $(SLB)$/idxf.lib # $(LB)$/rtftoken.lib

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=       $(SLO)$/idxf.obj
.ENDIF
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# ==========================================================================

.INCLUDE :  target.mk
