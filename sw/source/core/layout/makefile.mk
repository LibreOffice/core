#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
    $(SLO)$/anchoredobject.obj \
    $(SLO)$/atrfrm.obj \
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
    $(SLO)$/objstmpconsiderwrapinfl.obj \
    $(SLO)$/pagechg.obj \
    $(SLO)$/pagedesc.obj \
    $(SLO)$/paintfrm.obj \
    $(SLO)$/sectfrm.obj \
    $(SLO)$/softpagebreak.obj \
    $(SLO)$/sortedobjsimpl.obj \
    $(SLO)$/ssfrm.obj \
    $(SLO)$/tabfrm.obj \
    $(SLO)$/trvlfrm.obj \
    $(SLO)$/virtoutp.obj \
    $(SLO)$/wsfrm.obj

SLOFILES =  \
    $(EXCEPTIONSFILES) \
    $(SLO)$/objectformattertxtfrm.obj \
    $(SLO)$/objectformatterlayfrm.obj \
    $(SLO)$/sortedobjs.obj \
    $(SLO)$/swselectionlist.obj \
    $(SLO)$/unusedf.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
    pagefrm.src

.IF "$(DBG_LEVEL)">="2"
SLOFILES +=  \
        $(SLO)$/dbg_lay.obj
.ENDIF


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

