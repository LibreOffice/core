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
TARGET=basic_sbx

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

INC+= -I$(PRJ)$/inc$/bf_basic

# --- Allgemein -----------------------------------------------------------

EXCEPTIONSFILES=$(SLO)$/sbxarray.obj

SLOFILES=	\
    $(EXCEPTIONSFILES) \
    $(SLO)$/sbxbase.obj	\
    $(SLO)$/sbxvar.obj      \
    $(SLO)$/sbxvalue.obj    \
    $(SLO)$/sbxdec.obj \
    $(SLO)$/sbxint.obj  \
    $(SLO)$/sbxcurr.obj     \
    $(SLO)$/sbxstr.obj  \
    $(SLO)$/sbxbool.obj \
    $(SLO)$/sbxdbl.obj  \
    $(SLO)$/sbxdate.obj \
    $(SLO)$/sbxsng.obj  \
    $(SLO)$/sbxlng.obj  \
    $(SLO)$/sbxobj.obj  \
    $(SLO)$/sbxscan.obj     \
    $(SLO)$/sbxbyte.obj \
    $(SLO)$/sbxchar.obj \
    $(SLO)$/sbxulng.obj \
    $(SLO)$/sbxuint.obj \
    $(SLO)$/sbxcoll.obj \
    $(SLO)$/sbxres.obj      \
    $(SLO)$/sbxexec.obj \

#	$(SLO)$/sbxvals.obj	\
    $(SLO)$/sbxmstrm.obj	\


# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


