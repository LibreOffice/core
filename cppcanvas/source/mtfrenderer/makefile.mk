#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 05:14:24 $
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

PRJNAME=cppcanvas
TARGET=metafilerenderer
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

SLOFILES =	$(SLO)$/cachedprimitivebase.obj \
            $(SLO)$/bitmapaction.obj \
            $(SLO)$/implrenderer.obj \
            $(SLO)$/lineaction.obj \
            $(SLO)$/pointaction.obj \
            $(SLO)$/polypolyaction.obj \
            $(SLO)$/textaction.obj \
            $(SLO)$/transparencygroupaction.obj \
            $(SLO)$/mtftools.obj

# ==========================================================================

.INCLUDE :	target.mk
