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
PRJNAME=extensions
TARGET=scn
ENABLE_EXCEPTIONS=TRUE
PACKAGE=com$/sun$/star$/scanner
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
                $(SLO)$/scnserv.obj		\
                $(SLO)$/scanner.obj

.IF "$(GUI)"=="WNT"
SLOFILES+=\
                $(SLO)$/scanwin.obj		
.ENDIF

.IF "$(GUI)"=="UNX" || "$(GUI)" =="OS2"
SLOFILES+=\
                $(SLO)$/sane.obj		\
                $(SLO)$/sanedlg.obj		\
                $(SLO)$/scanunx.obj		\
                $(SLO)$/grid.obj

.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=\
    sanedlg.src		\
    grid.src

RESLIB1NAME=san
RESLIB1IMAGES=$(PRJ)$/source$/scanner
RESLIB1SRSFILES= $(SRS)$/scn.srs
RESLIB1DEPN= sanedlg.src sanedlg.hrc grid.src grid.hrc

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(SALLIB)			\
    $(ONELIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(SVTOOLLIB)

.IF "$(GUI)"=="UNX" && "$(OS)"!="FREEBSD" && \
    "$(OS)"!="NETBSD" && "$(OS)"!="OPENBSD" && "$(OS)"!="DRAGONFLY"
SHL1STDLIBS+=$(SVTOOLLIB) -ldl
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

