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
# $Revision: 1.13 $
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
PRJ=..$/..$

PRJNAME=svx
TARGET=intro

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

RSCLOCINC!:=$(RSCLOCINC);$(PRJ)$/RES

# setting the compiled by $user$ string in the About box
OOO_VENDOR*=$(USER)$(USERNAME)
RSCDEFS+=-DOOO_VENDOR="$(OOO_VENDOR)"

# --- Allgemein -----------------------------------------------------------

LOCALIZE_ME=intro_tmpl.hrc

SRS1NAME= iso
SRC1FILES=	\
    $(SRS1NAME).src

RESLIB1NAME=$(SRS1NAME)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
    $(SRS)$/$(SRS1NAME).srs

# Version "OpenOffice.org"
SRS2NAME= ooo
SRC2FILES=  \
    $(SRS2NAME).src

RESLIB2NAME=$(SRS2NAME)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES= \
    $(SRS)$/$(SRS2NAME).srs

.INCLUDE :  target.mk

$(SRS)$/iso.srs: $(INCCOM)$/intro.hrc
$(SRS)$/ooo.srs: $(INCCOM)$/intro.hrc

