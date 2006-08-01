#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2006-08-01 10:28:19 $
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

PRJ=..$/..

PRJNAME=vcl
TARGET=kdeplug
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# workaround for makedepend hang
MKDEPENDSOLVER=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# For some of the included external KDE headers, GCC complains about shadowed
# symbols in instantiated template code only at the end of a compilation unit,
# so the only solution is to disable that warning here:
.IF "$(COM)" == "GCC"
CFLAGSCXX+=-Wno-shadow
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

.IF "$(ENABLE_KDE)" != ""

CFLAGS+=$(KDE_CFLAGS)

SLOFILES=\
    $(SLO)$/kdedata.obj	\
    $(SLO)$/salnativewidgets-kde.obj

.ELSE # "$(ENABLE_KDE)" != ""

dummy:
    @echo KDE disabled - nothing to build
.ENDIF
.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
