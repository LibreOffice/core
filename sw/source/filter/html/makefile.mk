#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:17:54 $
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
TARGET=html


# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/css1atr.obj \
        $(SLO)$/css1kywd.obj \
        $(SLO)$/htmlatr.obj \
        $(SLO)$/htmlbas.obj \
        $(SLO)$/htmlcss1.obj \
        $(SLO)$/htmlctxt.obj \
        $(SLO)$/htmldraw.obj \
        $(SLO)$/htmlfld.obj \
        $(SLO)$/htmlfldw.obj \
        $(SLO)$/htmlfly.obj \
        $(SLO)$/htmlflyt.obj \
        $(SLO)$/htmlform.obj \
        $(SLO)$/htmlforw.obj \
        $(SLO)$/htmlftn.obj \
        $(SLO)$/htmlgrin.obj \
        $(SLO)$/htmlnum.obj \
        $(SLO)$/htmlplug.obj \
        $(SLO)$/htmlsect.obj \
        $(SLO)$/htmltab.obj \
        $(SLO)$/htmltabw.obj \
        $(SLO)$/parcss1.obj \
        $(SLO)$/svxcss1.obj \
        $(SLO)$/swhtml.obj \
        $(SLO)$/wrthtml.obj \
        $(SLO)$/SwAppletImpl.obj \

EXCEPTIONSFILES = \
        $(SLO)$/htmlfld.obj \
        $(SLO)$/htmlplug.obj \
        $(SLO)$/htmlsect.obj \
        $(SLO)$/swhtml.obj \
        $(SLO)$/wrthtml.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

