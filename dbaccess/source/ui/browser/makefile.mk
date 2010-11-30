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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=browser

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk
# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
            $(SLO)$/AsyncronousLink.obj		\
            $(SLO)$/brwctrlr.obj		\
            $(SLO)$/brwview.obj			\
            $(SLO)$/dataview.obj		\
            $(SLO)$/dbexchange.obj		\
            $(SLO)$/dbloader.obj		\
            $(SLO)$/dbtreemodel.obj		\
            $(SLO)$/dbtreeview.obj		\
            $(SLO)$/dsEntriesNoExp.obj		\
            $(SLO)$/dsbrowserDnD.obj		\
            $(SLO)$/exsrcbrw.obj                \
            $(SLO)$/formadapter.obj		\
            $(SLO)$/genericcontroller.obj	\
            $(SLO)$/sbagrid.obj			\
            $(SLO)$/sbamultiplex.obj		\
            $(SLO)$/unodatbr.obj

SLOFILES =\
            $(EXCEPTIONSFILES)

SRS1NAME=$(TARGET)
SRC1FILES =  sbabrw.src	\
            sbagrid.src

# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


