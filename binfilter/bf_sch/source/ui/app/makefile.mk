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

PROJECTPCH=sch
PROJECTPCHSOURCE=$(BFPRJ)$/util$/sch_sch

PRJNAME=binfilter
TARGET=sch_app

NO_HIDS=TRUE

BMP_OUT=$(RES)
BMP_IN =$(BFPRJ)$/res

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sch
# --- Files --------------------------------------------------------

#IMGLST_SRS=$(SRS)$/app.srs

SRS1NAME=$(TARGET)
SRC1FILES =	sch_app.src		\
        sch_strings.src

DEPOBJFILES =	\
        $(SLO)$/sch_schlib.obj

SLOFILES =	\
        $(SLO)$/sch_schdll.obj	\
        $(SLO)$/sch_schmod.obj      \
        $(SLO)$/sch_schresid.obj	\
        $(SLO)$/sch_schopt.obj	\
        $(SLO)$/sch_globopt.obj

LIB2TARGET =	$(SLB)$/bf_ysch.lib
LIB2ARCHIV =	$(LB)$/libbf_ysch.a
LIB2OBJFILES  =	$(SLO)$/sch_schlib.obj


.IF "$(GUI)" == "WNT"
NOOPTFILES=\
    $(SLO)$/sch_schdll.obj
.ENDIF



# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/schlib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"libbf_sch$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"bf_sch$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SRS)$/sch_app.srs: $(PRJ)$/inc$/bf_svx$/globlmn.hrc

$(SLO)$/sch_schlib.obj : $(INCCOM)$/schlib.hxx


