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
TARGET=sc_tool

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sc
# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/sc_adiasync.obj \
        $(SLO)$/sc_appoptio.obj \
        $(SLO)$/sc_autoform.obj \
        $(SLO)$/sc_callform.obj \
        $(SLO)$/sc_cellform.obj \
        $(SLO)$/sc_chartarr.obj \
        $(SLO)$/sc_chartlis.obj \
        $(SLO)$/sc_chgtrack.obj \
        $(SLO)$/sc_chgviset.obj \
        $(SLO)$/sc_collect.obj  \
        $(SLO)$/sc_compiler.obj \
        $(SLO)$/sc_consoli.obj  \
        $(SLO)$/sc_dbcolect.obj \
        $(SLO)$/sc_ddelink.obj \
        $(SLO)$/sc_detfunc.obj  \
        $(SLO)$/sc_detdata.obj  \
        $(SLO)$/sc_docoptio.obj \
        $(SLO)$/sc_editutil.obj \
        $(SLO)$/sc_indexmap.obj \
        $(SLO)$/sc_interpr1.obj \
        $(SLO)$/sc_interpr2.obj \
        $(SLO)$/sc_interpr3.obj \
        $(SLO)$/sc_interpr4.obj \
        $(SLO)$/sc_interpr5.obj \
        $(SLO)$/sc_interpr6.obj \
        $(SLO)$/sc_progress.obj \
        $(SLO)$/sc_rangenam.obj \
        $(SLO)$/sc_rangelst.obj \
        $(SLO)$/sc_rangeutl.obj \
        $(SLO)$/sc_rechead.obj  \
        $(SLO)$/sc_refupdat.obj \
        $(SLO)$/sc_refdata.obj \
        $(SLO)$/sc_scmatrix.obj \
        $(SLO)$/sc_subtotal.obj \
        $(SLO)$/sc_token.obj \
        $(SLO)$/sc_unitconv.obj \
        $(SLO)$/sc_userlist.obj \
        $(SLO)$/sc_viewopti.obj \
        $(SLO)$/sc_inputopt.obj \
        $(SLO)$/sc_printopt.obj \
        $(SLO)$/sc_optutil.obj \
        $(SLO)$/sc_zforauto.obj \
        $(SLO)$/sc_hints.obj \
        $(SLO)$/sc_prnsave.obj \
        $(SLO)$/sc_addincol.obj \
        $(SLO)$/sc_addinlis.obj \
        $(SLO)$/sc_addinhelpid.obj \
        $(SLO)$/sc_rangeseq.obj \
        $(SLO)$/sc_refreshtimer.obj

EXCEPTIONSFILES= \
        $(SLO)$/sc_addincol.obj

# [kh] POWERPC compiler problem
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC"
NOOPTFILES= \
                $(SLO)$/sc_subtotal.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

