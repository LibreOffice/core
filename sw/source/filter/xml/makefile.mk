#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 10:08:22 $
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

PRJNAME=sw
TARGET=xml

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/swxml.obj \
        $(SLO)$/xmlimp.obj \
        $(SLO)$/wrtxml.obj \
        $(SLO)$/xmlexp.obj \
        $(SLO)$/xmltext.obj \
        $(SLO)$/xmltexte.obj \
        $(SLO)$/xmltexti.obj \
        $(SLO)$/xmltbli.obj \
        $(SLO)$/xmltble.obj \
        $(SLO)$/xmlfmt.obj \
        $(SLO)$/xmlfmte.obj \
        $(SLO)$/xmlmeta.obj \
        $(SLO)$/xmlscript.obj \
        $(SLO)$/xmlitemm.obj \
        $(SLO)$/xmlitemi.obj \
        $(SLO)$/xmliteme.obj \
        $(SLO)$/xmlithlp.obj \
        $(SLO)$/xmlbrsh.obj \
        $(SLO)$/xmlfonte.obj \
        $(SLO)$/XMLRedlineImportHelper.obj \
        $(SLO)$/xmlitem.obj	\
        $(SLO)$/xmlitmpr.obj	\
        $(SLO)$/xmlimpit.obj \
        $(SLO)$/xmlexpit.obj

EXCEPTIONSFILES= \
        $(SLO)$/swxml.obj \
        $(SLO)$/xmlimp.obj \
        $(SLO)$/wrtxml.obj \
        $(SLO)$/xmlexp.obj \
        $(SLO)$/xmltext.obj \
        $(SLO)$/xmltexti.obj \
        $(SLO)$/xmltexte.obj \
        $(SLO)$/xmltbli.obj \
        $(SLO)$/xmltble.obj \
        $(SLO)$/xmlfmt.obj \
        $(SLO)$/xmlfmte.obj \
        $(SLO)$/xmlmeta.obj \
        $(SLO)$/xmlscript.obj \
        $(SLO)$/xmlitemi.obj \
        $(SLO)$/xmliteme.obj \
        $(SLO)$/xmlithlp.obj \
        $(SLO)$/xmlbrsh.obj \
        $(SLO)$/xmlfonte.obj \
        $(SLO)$/XMLRedlineImportHelper.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
