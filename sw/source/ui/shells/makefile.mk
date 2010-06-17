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

PRJNAME=sw
TARGET=shells

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        shells.src

SLOFILES =  \
        $(SLO)$/langhelper.obj \
        $(SLO)$/annotsh.obj \
        $(SLO)$/basesh.obj \
        $(SLO)$/beziersh.obj \
        $(SLO)$/drawdlg.obj \
        $(SLO)$/drawsh.obj \
        $(SLO)$/drformsh.obj \
        $(SLO)$/drwbassh.obj \
        $(SLO)$/drwtxtex.obj \
        $(SLO)$/drwtxtsh.obj \
        $(SLO)$/frmsh.obj \
        $(SLO)$/grfsh.obj \
        $(SLO)$/grfshex.obj \
        $(SLO)$/mediash.obj \
        $(SLO)$/listsh.obj \
        $(SLO)$/olesh.obj \
        $(SLO)$/slotadd.obj \
        $(SLO)$/tabsh.obj \
        $(SLO)$/textdrw.obj \
        $(SLO)$/textfld.obj \
        $(SLO)$/textglos.obj \
        $(SLO)$/textidx.obj \
        $(SLO)$/textsh.obj \
        $(SLO)$/textsh1.obj \
        $(SLO)$/textsh2.obj \
        $(SLO)$/txtattr.obj \
        $(SLO)$/txtcrsr.obj \
        $(SLO)$/txtnum.obj

EXCEPTIONSFILES =  \
        $(SLO)$/basesh.obj \
        $(SLO)$/annotsh.obj \
        $(SLO)$/drwtxtsh.obj \
        $(SLO)$/textsh.obj \
        $(SLO)$/textsh2.obj \
        $(SLO)$/grfshex.obj \
        $(SLO)$/drawsh.obj \
        $(SLO)$/drwtxtsh.obj \
        $(SLO)$/frmsh.obj \
        $(SLO)$/drwtxtex.obj \
        $(SLO)$/slotadd.obj \
        $(SLO)$/textsh1.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/shells.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

