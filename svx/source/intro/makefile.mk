#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2005-09-28 15:44:58 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
PRJ=..$/..$

PRJNAME=svx
TARGET=intro

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk


RSCLOCINC!:=$(RSCLOCINC);$(PRJ)$/RES

# setting the compiled by $user$ using $license$ string in the about box

.IF "$(BUILD_SPECIAL)"==""
RSCDEFS+=-DOOO_VENDOR="$(USER)$(USERNAME)"  -DOOO_LICENSE="LGPL"
.ELSE
RSCDEFS+=-DBUILD_SPECIAL=True -DOOO_LICENSE="LGPL"
.ENDIF

# --- Allgemein -----------------------------------------------------------

LOCALIZE_ME=intro_tmpl.hrc

SRS1NAME= iso
SRC1FILES=	\
    $(SRS1NAME).src

RESLIB1NAME=$(SRS1NAME)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
    $(SRS)$/$(SRS1NAME).srs

# Version "OpenOffice"
SRS4NAME= ooo
SRC4FILES=	\
    $(SRS4NAME).src

RESLIB4NAME=$(SRS4NAME)
RESLIB4IMAGES=$(PRJ)$/res
RESLIB4SRSFILES= \
    $(SRS)$/$(SRS4NAME).srs

.INCLUDE :  target.mk

$(SRS)$/iso.srs: $(INCCOM)$/intro.hrc
$(SRS)$/ooo.srs: $(INCCOM)$/intro.hrc
