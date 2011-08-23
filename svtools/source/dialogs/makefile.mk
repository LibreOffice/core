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

PRJ=..$/..

PRJNAME=svtools
TARGET=dialogs

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=		filedlg2.src	\
                                so3res.src    \
                                formats.src    \
                prnsetup.src	\
                printdlg.src	\
                colrdlg.src		\
                addresstemplate.src	\
                wizardmachine.src


EXCEPTIONSFILES=	$(SLO)$/addresstemplate.obj	\
                    $(SLO)$/colctrl.obj 	\
                    $(SLO)$/colrdlg.obj 	\
                    $(SLO)$/filedlg.obj 	\
                    $(SLO)$/filedlg2.obj	\
                    $(SLO)$/insdlg.obj		\
                    $(SLO)$/printdlg.obj	\
                    $(SLO)$/prnsetup.obj	\
                    $(SLO)$/property.obj	\
                    $(SLO)$/roadmapwizard.obj 	\
                    $(SLO)$/wizardmachine.obj 	\
                    $(SLO)$/wizdlg.obj

SLOFILES=	\
                    $(SLO)$/addresstemplate.obj \
                    $(SLO)$/colctrl.obj 	\
                    $(SLO)$/colrdlg.obj 	\
                    $(SLO)$/filedlg.obj 	\
                    $(SLO)$/filedlg2.obj	\
                    $(SLO)$/insdlg.obj		\
                    $(SLO)$/mcvmath.obj		\
                    $(SLO)$/printdlg.obj	\
                    $(SLO)$/prnsetup.obj	\
                    $(SLO)$/property.obj	\
                    $(SLO)$/roadmapwizard.obj 	\
                    $(SLO)$/wizardmachine.obj 	\
                    $(SLO)$/wizdlg.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
