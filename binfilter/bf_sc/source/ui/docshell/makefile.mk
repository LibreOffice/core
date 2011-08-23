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
TARGET=sc_docshell

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sc
# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/sc_docsh.obj	\
        $(SLO)$/sc_docsh2.obj	\
        $(SLO)$/sc_docsh3.obj	\
        $(SLO)$/sc_docsh4.obj   	\
        $(SLO)$/sc_docsh5.obj   	\
        $(SLO)$/sc_docsh6.obj   	\
        $(SLO)$/sc_tablink.obj   	\
        $(SLO)$/sc_arealink.obj   	\
        $(SLO)$/sc_dbdocfun.obj 	\
        $(SLO)$/sc_dbdocimp.obj 	\
        $(SLO)$/sc_impex.obj	\
        $(SLO)$/sc_docfunc.obj	\
        $(SLO)$/sc_olinefun.obj	\
        $(SLO)$/sc_pntlock.obj \
        $(SLO)$/sc_sizedev.obj \
        $(SLO)$/sc_editable.obj


EXCEPTIONSFILES= \
        $(SLO)$/sc_dbdocimp.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

