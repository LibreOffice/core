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
TARGET=access

# --- Settings -----------------------------------------------------
#ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/acccell.obj \
        $(SLO)$/acccontext.obj \
        $(SLO)$/accdoc.obj \
        $(SLO)$/accembedded.obj \
        $(SLO)$/accfootnote.obj \
        $(SLO)$/accframe.obj \
        $(SLO)$/accframebase.obj \
        $(SLO)$/accfrmobj.obj \
        $(SLO)$/accfrmobjmap.obj \
        $(SLO)$/accfrmobjslist.obj \
        $(SLO)$/accgraphic.obj \
        $(SLO)$/accheaderfooter.obj \
        $(SLO)$/acchyperlink.obj \
        $(SLO)$/acchypertextdata.obj \
        $(SLO)$/accmap.obj \
        $(SLO)$/accnotextframe.obj \
        $(SLO)$/accpage.obj \
        $(SLO)$/accpara.obj \
        $(SLO)$/accportions.obj \
        $(SLO)$/accpreview.obj \
        $(SLO)$/accselectionhelper.obj \
        $(SLO)$/acctable.obj \
                $(SLO)$/acctextframe.obj \
                $(SLO)$/textmarkuphelper.obj \
                $(SLO)$/parachangetrackinginfo.obj

EXCEPTIONSFILES=	\
        $(SLO)$/acccell.obj \
        $(SLO)$/acccontext.obj \
        $(SLO)$/accdoc.obj \
        $(SLO)$/accembedded.obj \
        $(SLO)$/accfootnote.obj \
        $(SLO)$/accframe.obj \
        $(SLO)$/accframebase.obj \
        $(SLO)$/accfrmobj.obj \
        $(SLO)$/accfrmobjmap.obj \
        $(SLO)$/accfrmobjslist.obj \
        $(SLO)$/accgraphic.obj \
        $(SLO)$/accheaderfooter.obj \
        $(SLO)$/acchyperlink.obj \
        $(SLO)$/acchypertextdata.obj \
        $(SLO)$/accmap.obj \
        $(SLO)$/accnotextframe.obj \
        $(SLO)$/accpage.obj \
        $(SLO)$/accpara.obj \
        $(SLO)$/accportions.obj \
        $(SLO)$/accpreview.obj \
        $(SLO)$/accselectionhelper.obj \
        $(SLO)$/acctable.obj \
                $(SLO)$/acctextframe.obj \
                $(SLO)$/textmarkuphelper.obj \
                $(SLO)$/parachangetrackinginfo.obj

# --- Targets -------------------------------------------------------


.INCLUDE :	target.mk

