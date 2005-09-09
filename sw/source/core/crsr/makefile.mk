#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 03:05:31 $
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
TARGET=crsr

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
        bookmrk.cxx \
        callnk.cxx \
        crbm.cxx \
        crsrsh.cxx \
        crstrvl.cxx \
        crstrvl1.cxx \
        findattr.cxx \
        findcoll.cxx \
        findfmt.cxx \
        findtxt.cxx \
        pam.cxx \
        paminit.cxx \
        swcrsr.cxx \
        trvlcol.cxx \
        trvlfnfl.cxx \
        trvlreg.cxx \
        trvltbl.cxx \
        unocrsr.cxx \
        viscrs.cxx

.IF "$(GUI)"=="MAC"
CXXFILES += \
        crsrsh1.cxx
.ENDIF
.IF "$(mydebug)" != ""
CXXFILES += \
        pamio.cxx
.ENDIF

SLOFILES =	\
        $(SLO)$/bookmrk.obj \
        $(SLO)$/callnk.obj \
        $(SLO)$/crbm.obj \
        $(SLO)$/crsrsh.obj \
        $(SLO)$/crstrvl.obj \
        $(SLO)$/crstrvl1.obj \
        $(SLO)$/findattr.obj \
        $(SLO)$/findcoll.obj \
        $(SLO)$/findfmt.obj \
        $(SLO)$/findtxt.obj \
        $(SLO)$/pam.obj \
        $(SLO)$/paminit.obj \
        $(SLO)$/swcrsr.obj \
        $(SLO)$/trvlcol.obj \
        $(SLO)$/trvlfnfl.obj \
        $(SLO)$/trvlreg.obj \
        $(SLO)$/trvltbl.obj \
        $(SLO)$/unocrsr.obj \
        $(SLO)$/viscrs.obj

.IF "$(GUI)"=="MAC"
SLOFILES +=  \
        $(SLO)$/crsrsh1.obj
.ENDIF
.IF "$(mydebug)" != ""
SLOFILES +=  \
        $(SLO)$/pamio.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk
