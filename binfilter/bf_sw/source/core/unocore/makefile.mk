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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME=binfilter
TARGET=sw_unocore

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw

.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF



# --- Files --------------------------------------------------------
EXCEPTIONSFILES=	\
        $(SLO)$/sw_swunohelper.obj\
        $(SLO)$/sw_SwXTextDefaults.obj\
    $(SLO)$/sw_unobkm.obj\
    $(SLO)$/sw_unoevent.obj\
    $(SLO)$/sw_unocrsrhelper.obj\
    $(SLO)$/sw_unoevtlstnr.obj\
    $(SLO)$/sw_unoftn.obj\
    $(SLO)$/sw_unorefmk.obj\
    $(SLO)$/sw_unosect.obj\
    $(SLO)$/sw_unosett.obj\
    $(SLO)$/sw_unocoll.obj\
    $(SLO)$/sw_unodraw.obj\
    $(SLO)$/sw_unofield.obj\
    $(SLO)$/sw_unoframe.obj\
    $(SLO)$/sw_unoidx.obj\
    $(SLO)$/sw_unoobj.obj\
    $(SLO)$/sw_unoobj2.obj\
    $(SLO)$/sw_unoparagraph.obj\
    $(SLO)$/sw_unoport.obj\
    $(SLO)$/sw_unoredline.obj\
    $(SLO)$/sw_unoredlines.obj\
    $(SLO)$/sw_unosrch.obj\
    $(SLO)$/sw_unostyle.obj\
    $(SLO)$/sw_unotbl.obj \
    $(SLO)$/sw_TextCursorHelper.obj \
    $(SLO)$/sw_unotext.obj

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
EXCEPTIONSNOOPTFILES =$(SLO)$/sw_unoportenum.obj
.ELSE
EXCEPTIONSFILES +=$(SLO)$/sw_unoportenum.obj
.ENDIF


SLOFILES =	\
        $(SLO)$/sw_swunohelper.obj\
        $(SLO)$/sw_SwXTextDefaults.obj\
        $(SLO)$/sw_unoportenum.obj\
    $(SLO)$/sw_unobkm.obj\
    $(SLO)$/sw_unoevent.obj\
    $(SLO)$/sw_unocrsrhelper.obj\
    $(SLO)$/sw_unoevtlstnr.obj\
    $(SLO)$/sw_unoftn.obj\
    $(SLO)$/sw_unorefmk.obj\
    $(SLO)$/sw_unosect.obj\
    $(SLO)$/sw_unosett.obj\
    $(SLO)$/sw_unocoll.obj\
    $(SLO)$/sw_unodraw.obj\
    $(SLO)$/sw_unofield.obj\
    $(SLO)$/sw_unoframe.obj\
    $(SLO)$/sw_unoidx.obj\
    $(SLO)$/sw_unoobj.obj\
    $(SLO)$/sw_unoobj2.obj\
    $(SLO)$/sw_unoparagraph.obj\
    $(SLO)$/sw_unoport.obj\
    $(SLO)$/sw_unoredline.obj\
    $(SLO)$/sw_unoredlines.obj\
    $(SLO)$/sw_unosrch.obj\
    $(SLO)$/sw_unostyle.obj\
    $(SLO)$/sw_unotbl.obj \
    $(SLO)$/sw_TextCursorHelper.obj \
    $(SLO)$/sw_unotext.obj\
        $(SLO)$/sw_unoclbck.obj\
        $(SLO)$/sw_unomap.obj\
        $(SLO)$/sw_unoprnms.obj



# --- Targets -------------------------------------------------------


.INCLUDE :	target.mk

