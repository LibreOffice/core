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

PRJNAME=binfilter
TARGET=basic_runtime

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

INC+= -I$(PRJ)$/inc$/bf_basic

# --- Allgemein -----------------------------------------------------------

SLOFILES=	\
    $(SLO)$/basrdll.obj	\

#	$(SLO)$/inputbox.obj	\
    $(SLO)$/runtime.obj	\
    $(SLO)$/step0.obj	\
    $(SLO)$/step1.obj	\
    $(SLO)$/step2.obj	\
    $(SLO)$/iosys.obj	\
    $(SLO)$/stdobj.obj	\
    $(SLO)$/stdobj1.obj	\
    $(SLO)$/methods.obj	\
    $(SLO)$/methods1.obj	\
    $(SLO)$/props.obj	\
    $(SLO)$/ddectrl.obj	\
    $(SLO)$/dllmgr.obj

.IF "$(GUI)$(CPU)" == "WINI"
#SLOFILES+=	$(SLO)$/win.obj
.ENDIF

.IF "$(GUI)$(CPU)" == "WNTI"
#SLOFILES+=	$(SLO)$/wnt.obj
.ENDIF

.IF "$(GUI)$(CPU)" == "OS2I"
#SLOFILES+= $(SLO)$/os2.obj
.ENDIF

#EXCEPTIONSFILES=$(SLO)$/step0.obj	\
        $(SLO)$/step2.obj	\
        $(SLO)$/methods.obj	\
        $(SLO)$/methods1.obj	\
        $(SLO)$/iosys.obj	\
        $(SLO)$/runtime.obj

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

