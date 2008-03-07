#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 14:56:25 $
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
TARGET=layout

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

.IF "$(madebug)" != ""
CDEFS+=-DDEBUG
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = \
    $(SLO)$/anchoreddrawobject.obj \
    $(SLO)$/flycnt.obj \
    $(SLO)$/laycache.obj \
    $(SLO)$/layouter.obj \
    $(SLO)$/movedfwdfrmsbyobjpos.obj \
    $(SLO)$/objectformatter.obj \
    $(SLO)$/objstmpconsiderwrapinfl.obj \
    $(SLO)$/paintfrm.obj \
    $(SLO)$/softpagebreak.obj \
    $(SLO)$/sortedobjsimpl.obj \
    $(SLO)$/pagechg.obj


SLOFILES =  \
    $(SLO)$/atrfrm.obj \
    $(SLO)$/anchoredobject.obj \
    $(SLO)$/anchoreddrawobject.obj \
    $(SLO)$/calcmove.obj \
    $(SLO)$/colfrm.obj \
    $(SLO)$/findfrm.obj \
    $(SLO)$/flowfrm.obj \
    $(SLO)$/fly.obj \
    $(SLO)$/flycnt.obj \
    $(SLO)$/flyincnt.obj \
    $(SLO)$/flylay.obj \
    $(SLO)$/flypos.obj \
    $(SLO)$/frmtool.obj \
    $(SLO)$/ftnfrm.obj \
    $(SLO)$/hffrm.obj \
    $(SLO)$/layact.obj \
    $(SLO)$/laycache.obj \
    $(SLO)$/layouter.obj \
    $(SLO)$/movedfwdfrmsbyobjpos.obj \
    $(SLO)$/newfrm.obj \
    $(SLO)$/objectformatter.obj \
    $(SLO)$/objectformattertxtfrm.obj \
    $(SLO)$/objectformatterlayfrm.obj \
    $(SLO)$/objstmpconsiderwrapinfl.obj \
    $(SLO)$/pagechg.obj \
    $(SLO)$/pagedesc.obj \
    $(SLO)$/paintfrm.obj \
    $(SLO)$/sectfrm.obj \
    $(SLO)$/softpagebreak.obj \
    $(SLO)$/sortedobjs.obj \
    $(SLO)$/sortedobjsimpl.obj \
    $(SLO)$/ssfrm.obj \
    $(SLO)$/swselectionlist.obj \
    $(SLO)$/tabfrm.obj \
    $(SLO)$/trvlfrm.obj \
    $(SLO)$/unusedf.obj \
    $(SLO)$/virtoutp.obj \
    $(SLO)$/wsfrm.obj

.IF "$(product)"==""
.IF "$(cap)"==""
SLOFILES +=  \
        $(SLO)$/dbg_lay.obj
.ENDIF
.ENDIF


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

