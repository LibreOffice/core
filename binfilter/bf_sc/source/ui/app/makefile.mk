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
TARGET=sc_app

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sc
# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/sc_scmod.obj  \
    $(SLO)$/sc_scmod2.obj  \
    $(SLO)$/sc_scdll.obj  \
    $(SLO)$/sc_rfindlst.obj \
    $(SLO)$/sc_uiitems.obj  \
    $(SLO)$/sc_msgpool.obj

EXCEPTIONSFILES= \
    $(SLO)$/sc_scmod2.obj \
    $(SLO)$/sc_scmod.obj

DEPOBJFILES=$(SLO)$/sc_sclib.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(DEPOBJFILES)

$(SLO)$/sc_sclib.obj :	$(INCCOM)$/scdll0.hxx
#$(OBJ)$/sc_sclib.obj :	$(INCCOM)$/scdll0.hxx

$(INCCOM)$/scdll0.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    echo \#define DLL_NAME \"libbf_sc$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE			# "$(GUI)"=="UNX"
    echo \#define DLL_NAME \"bf_sc$(DLLPOSTFIX).DLL\" >$@
.ENDIF			# "$(GUI)"=="UNX"

$(SRS)$/sc_app.srs: $(PRJ)$/inc$/bf_svx$/globlmn.hrc

