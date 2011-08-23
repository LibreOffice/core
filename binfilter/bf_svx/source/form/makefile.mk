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
TARGET=svx_form

NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Files --------------------------------------------------------

BMP_IN=$(BFPRJ)$/win$/res

SRS1NAME=svx_form
SRC1FILES= \
    svx_fmexpl.src 		\
    svx_filtnav.src		\
    svx_taborder.src 	\
    svx_fmstring.src 	\
    svx_tbxform.src		\
    svx_fmsearch.src


SLOFILES = \
    $(SLO)$/svx_fmpage.obj      \
    $(SLO)$/svx_fmmodel.obj		\
    $(SLO)$/svx_fmdpage.obj		\
    $(SLO)$/svx_fmobj.obj		\
    $(SLO)$/svx_fmobjfac.obj \
    $(SLO)$/svx_fmimplids.obj \
    $(SLO)$/svx_fmdmod.obj \
    $(SLO)$/svx_ParseContext.obj \
    $(SLO)$/svx_siimport.obj \
    $(SLO)$/svx_fmtools.obj \
    $(SLO)$/svx_fmpgeimp.obj

.IF "$(OS)$(CPU)"=="SOLARISI" 
NOOPTFILES=$(SLO)$/svx_fmprop.obj
.ENDIF

.INCLUDE :  target.mk
