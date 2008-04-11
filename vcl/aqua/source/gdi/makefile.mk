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
# $Revision: 1.15 $
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

PRJNAME=vcl
TARGET=salgdi
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="aqua"

SLOFILES=   $(SLO)$/salmathutils.obj      \
            $(SLO)$/salcolorutils.obj     \
            $(SLO)$/salpixmaputils.obj    \
            $(SLO)$/salgdiutils.obj       \
            $(SLO)$/salnativewidgets.obj  \
            $(SLO)$/salatsuifontutils.obj \
            $(SLO)$/salatslayout.obj      \
            $(SLO)$/salgdi.obj            \
            $(SLO)$/salvd.obj             \
            $(SLO)$/salprn.obj            \
            $(SLO)$/aquaprintview.obj     \
            $(SLO)$/salbmp.obj            \
            $(SLO)$/salogl.obj

.IF "$(ENABLE_CAIRO)" == "TRUE"
CDEFS+= -DCAIRO
.ENDIF

.ENDIF		# "$(GUIBASE)"!="aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

