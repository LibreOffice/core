#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.21 $
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
TARGET=unocore

# --- Settings -----------------------------------------------------
#ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------
EXCEPTIONSFILES=	\
    $(SLO)$/swunohelper.obj\
    $(SLO)$/SwXTextDefaults.obj\
    $(SLO)$/unobkm.obj\
    $(SLO)$/unochart.obj\
    $(SLO)$/unoevent.obj\
    $(SLO)$/unocrsrhelper.obj\
    $(SLO)$/unoevtlstnr.obj\
    $(SLO)$/unoftn.obj\
    $(SLO)$/unorefmk.obj\
    $(SLO)$/unosect.obj\
    $(SLO)$/unosett.obj\
    $(SLO)$/unocoll.obj\
    $(SLO)$/unodraw.obj\
    $(SLO)$/unofield.obj\
    $(SLO)$/unoframe.obj\
    $(SLO)$/unoidx.obj\
    $(SLO)$/unoobj.obj\
    $(SLO)$/unoobj2.obj\
    $(SLO)$/unoparagraph.obj\
    $(SLO)$/unoport.obj\
    $(SLO)$/unoredline.obj\
    $(SLO)$/unoredlines.obj\
    $(SLO)$/unosrch.obj\
    $(SLO)$/unostyle.obj\
    $(SLO)$/unotbl.obj \
    $(SLO)$/unoflatpara.obj\
    $(SLO)$/unotextmarkup.obj\
    $(SLO)$/TextCursorHelper.obj \
    $(SLO)$/unotext.obj

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
EXCEPTIONSNOOPTFILES =$(SLO)$/unoportenum.obj
.ELSE
EXCEPTIONSFILES +=$(SLO)$/unoportenum.obj
.ENDIF


SRS1NAME=$(TARGET)
SRC1FILES =  \
        unocore.src

SLOFILES =	\
    $(SLO)$/swunohelper.obj\
    $(SLO)$/SwXTextDefaults.obj\
    $(SLO)$/unoportenum.obj\
    $(SLO)$/unobkm.obj\
    $(SLO)$/unochart.obj\
    $(SLO)$/unoevent.obj\
    $(SLO)$/unocrsrhelper.obj\
    $(SLO)$/unoevtlstnr.obj\
    $(SLO)$/unoftn.obj\
    $(SLO)$/unorefmk.obj\
    $(SLO)$/unosect.obj\
    $(SLO)$/unosett.obj\
    $(SLO)$/unocoll.obj\
    $(SLO)$/unodraw.obj\
    $(SLO)$/unofield.obj\
    $(SLO)$/unoframe.obj\
    $(SLO)$/unoidx.obj\
    $(SLO)$/unoobj.obj\
    $(SLO)$/unoobj2.obj\
    $(SLO)$/unoparagraph.obj\
    $(SLO)$/unoport.obj\
    $(SLO)$/unoredline.obj\
    $(SLO)$/unoredlines.obj\
        $(SLO)$/unosrch.obj\
    $(SLO)$/unostyle.obj\
    $(SLO)$/unotbl.obj \
        $(SLO)$/unoflatpara.obj\
        $(SLO)$/unotextmarkup.obj\
        $(SLO)$/TextCursorHelper.obj \
    $(SLO)$/unotext.obj\
    $(SLO)$/unomap.obj\
    $(SLO)$/unoprnms.obj\
    $(SLO)$/XMLRangeHelper.obj



# --- Targets -------------------------------------------------------


.INCLUDE :	target.mk

