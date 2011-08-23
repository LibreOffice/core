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
TARGET=sw_app

NO_HIDS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk

INC+= -I$(PRJ)$/inc$/bf_sw
RSCUPDVER=$(RSCREVISION)(SV$(UPD)$(UPDMINOR))

#IMGLST_SRS=$(SRS)$/app.srs
#BMP_IN=$(PRJ)$/win/imglst

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =\
        sw_app.src     \
        sw_error.src

SLOFILES = \
        $(SLO)$/sw_apphdl.obj   \
        $(SLO)$/sw_docsh.obj    \
        $(SLO)$/sw_docsh2.obj   \
        $(SLO)$/sw_docshdrw.obj \
        $(SLO)$/sw_docshini.obj \
        $(SLO)$/sw_docstyle.obj \
        $(SLO)$/sw_mainwn.obj   \
        $(SLO)$/sw_version.obj  \
        $(SLO)$/sw_swmodule.obj \
        $(SLO)$/sw_swmodul1.obj \
        $(SLO)$/sw_swdll.obj


EXCEPTIONSFILES= \
        $(SLO)$/sw_swmodule.obj \
        $(SLO)$/sw_swmodul1.obj \
        $(SLO)$/sw_docsh2.obj


DEPOBJFILES= \
    $(SLO)$/sw_swlib.obj \
    $(SLO)$/sw_swcomlib.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(DEPOBJFILES)

$(SLO)$/sw_swdll.obj : $(INCCOM)$/swdll0.hxx
$(SLO)$/sw_swlib.obj : $(INCCOM)$/swdll0.hxx
$(SLO)$/sw_swmodule.obj : $(INCCOM)$/swdll0.hxx

$(INCCOM)$/swdll0.hxx: makefile.mk
    echo \#define DLL_NAME \"$(DLLPRE)bf_sw$(DLLPOSTFIX)$(DLLPOST)\" >$@

$(SRS)$/sw_app.srs: $(PRJ)$/inc$/bf_svx$/globlmn.hrc

