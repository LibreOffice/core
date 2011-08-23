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

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=svx_outliner

NO_HIDS=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=outl_pch
PROJECTPCHSOURCE=svx_outl_pch


# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Allgemein ----------------------------------------------------------

SLOFILES=	\
            $(SLO)$/svx_outliner.obj 	\
            $(SLO)$/svx_outlin2.obj		\
            $(SLO)$/svx_paralist.obj 	\
            $(SLO)$/svx_outlvw.obj 		\
            $(SLO)$/svx_outleeng.obj 	\
            $(SLO)$/svx_outlobj.obj
            
SRS1NAME=$(TARGET)
SRC1FILES=	svx_outliner.src

.INCLUDE :  target.mk
