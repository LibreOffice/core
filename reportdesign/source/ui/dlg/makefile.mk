#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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
PRJNAME=reportdesign
TARGET=uidlg
IMGLST_SRS=$(SRS)$/$(TARGET).srs
BMP_IN=$(PRJ)$/res

VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
# .INCLUDE : $(PRJ)$/util$/dll.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        dlgpage.src	\
        PageNumber.src	\
        DateTime.src	\
        CondFormat.src	\
        Navigator.src	\
        GroupsSorting.src

# ... object files ............................

EXCEPTIONSFILES=	\
        $(SLO)$/dlgpage.obj					\
        $(SLO)$/Condition.obj				\
        $(SLO)$/CondFormat.obj				\
        $(SLO)$/GroupExchange.obj			\
        $(SLO)$/PageNumber.obj				\
        $(SLO)$/DateTime.obj				\
        $(SLO)$/AddField.obj				\
        $(SLO)$/Navigator.obj				\
        $(SLO)$/GroupsSorting.obj           \
        $(SLO)$/Formula.obj
        
SLOFILES=	\
        $(EXCEPTIONSFILES)					\


# --- Targets ----------------------------------

.INCLUDE : target.mk

