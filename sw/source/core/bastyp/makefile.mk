#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:03:16 $
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
TARGET=bastyp

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

CXXFILES = \
        SwBitArray.cxx \
        bparr.cxx \
        breakit.cxx \
        calc.cxx \
                checkit.cxx \
        index.cxx \
        init.cxx \
        ring.cxx \
        swcache.cxx \
        swrect.cxx \
        swregion.cxx \
        swtypes.cxx \
        tabcol.cxx \
        SwSmartTagMgr.cxx

SLOFILES =  \
        $(SLO)$/SwBitArray.obj \
        $(SLO)$/bparr.obj \
        $(SLO)$/breakit.obj \
        $(SLO)$/calc.obj \
                $(SLO)$/checkit.obj \
        $(SLO)$/index.obj \
        $(SLO)$/init.obj \
        $(SLO)$/ring.obj \
        $(SLO)$/swcache.obj \
        $(SLO)$/swrect.obj \
        $(SLO)$/swregion.obj \
        $(SLO)$/swtypes.obj \
        $(SLO)$/tabcol.obj \
        $(SLO)$/SwSmartTagMgr.obj

EXCEPTIONSFILES = \
        $(SLO)$/SwBitArray.obj \
        $(SLO)$/tabcol.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

