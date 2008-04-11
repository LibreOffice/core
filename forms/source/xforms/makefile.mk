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
# $Revision: 1.5 $
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

PRJ=..$/..

PRJNAME=forms
TARGET=xforms

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/model.obj \
        $(SLO)$/model_ui.obj \
        $(SLO)$/binding.obj \
        $(SLO)$/xforms_services.obj \
        $(SLO)$/unohelper.obj \
        $(SLO)$/propertysetbase.obj \
        $(SLO)$/pathexpression.obj \
        $(SLO)$/computedexpression.obj \
        $(SLO)$/boolexpression.obj \
        $(SLO)$/mip.obj \
        $(SLO)$/submission.obj \
        $(SLO)$/datatyperepository.obj \
        $(SLO)$/datatypes.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/enumeration.obj \
        $(SLO)$/resourcehelper.obj \
        $(SLO)$/xmlhelper.obj \
        $(SLO)$/xformsevent.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

