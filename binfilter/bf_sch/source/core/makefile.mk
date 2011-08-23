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

PROJECTPCH=sch
PROJECTPCHSOURCE=$(BFPRJ)$/util$/sch_sch

PRJNAME=binfilter
TARGET=sch_core

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sch
# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = sch_glob.src

SLOFILES =  \
    $(SLO)$/sch_chtscene.obj	\
    $(SLO)$/sch_globfunc.obj	\
    $(SLO)$/sch_chtmodel.obj	\
    $(SLO)$/sch_chtmode1.obj	\
    $(SLO)$/sch_chtmode2.obj	\
    $(SLO)$/sch_chtmode3.obj	\
    $(SLO)$/sch_chtmode4.obj	\
    $(SLO)$/sch_chtmode5.obj	\
    $(SLO)$/sch_chtmode6.obj	\
    $(SLO)$/sch_chtmode7.obj	\
    $(SLO)$/sch_chtmode8.obj	\
    $(SLO)$/sch_charttyp.obj	\
    $(SLO)$/sch_chtmode9.obj	\
    $(SLO)$/sch_chartbar.obj	\
    $(SLO)$/sch_chtmod2a.obj	\
    $(SLO)$/sch_chtmod3d.obj	\
    $(SLO)$/sch_chtm3d2.obj		\
    $(SLO)$/sch_chartdoc.obj	\
    $(SLO)$/sch_stlsheet.obj	\
    $(SLO)$/sch_schgroup.obj	\
    $(SLO)$/sch_stlpool.obj		\
    $(SLO)$/sch_objfac.obj		\
    $(SLO)$/sch_axisid.obj		\
    $(SLO)$/sch_objid.obj		\
    $(SLO)$/sch_objadj.obj		\
    $(SLO)$/sch_datarow.obj		\
    $(SLO)$/sch_datapoin.obj	\
    $(SLO)$/sch_schiocmp.obj	\
    $(SLO)$/sch_itempool.obj	\
    $(SLO)$/sch_datalog.obj		\
    $(SLO)$/sch_memchrt.obj 	\
    $(SLO)$/sch_chdescr.obj 	\
    $(SLO)$/sch_chaxis.obj		\
    $(SLO)$/sch_calculat.obj	\
    $(SLO)$/sch_axisobj.obj		\
    $(SLO)$/sch_SchAddInCollection.obj	\
    $(SLO)$/sch_ReBuildHint.obj

.IF "$(GUI)" == "WIN"
NOOPTFILES=\
    $(OBJ)$/sch_chtmode5.obj
.ENDIF

EXCEPTIONSFILES= \
    $(SLO)$/sch_chtmode4.obj   \
    $(SLO)$/sch_chtmodel.obj   \
    $(SLO)$/sch_SchAddInCollection.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

