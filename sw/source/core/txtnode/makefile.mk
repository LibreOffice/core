#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2006-11-22 11:46:56 $
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
TARGET=txtnode

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/atrfld.obj \
        $(SLO)$/atrflyin.obj \
        $(SLO)$/atrftn.obj \
        $(SLO)$/atrref.obj \
        $(SLO)$/atrtox.obj \
        $(SLO)$/chrfmt.obj \
        $(SLO)$/fmtatr1.obj \
        $(SLO)$/fmtatr2.obj \
        $(SLO)$/fntcap.obj \
        $(SLO)$/fntcache.obj \
        $(SLO)$/swfntcch.obj \
        $(SLO)$/ndhints.obj \
        $(SLO)$/ndtxt.obj \
        $(SLO)$/swfont.obj \
        $(SLO)$/thints.obj \
        $(SLO)$/txatbase.obj \
        $(SLO)$/txatritr.obj \
        $(SLO)$/txtatr2.obj \
                $(SLO)$/txtedt.obj \
                $(SLO)$/txtnodenumattr.obj

.IF "$(dbutil)" != ""
OFILES+=$(SLO)$/dbchratr.$(QBJX)
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

