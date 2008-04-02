#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2008-04-02 09:50:26 $
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

