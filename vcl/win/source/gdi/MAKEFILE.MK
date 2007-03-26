#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: MAKEFILE.MK,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 14:40:44 $
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

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- #105371#
CFLAGS += -DWINVER=0x0400

.IF "$(COM)"=="GCC"
CFLAGS += -D_WIN32_WINNT=0x0501
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/salgdi.obj   \
            $(SLO)$/salgdi2.obj  \
            $(SLO)$/salgdi3.obj  \
            $(SLO)$/salvd.obj    \
            $(SLO)$/salprn.obj   \
            $(SLO)$/salbmp.obj	 \
            $(SLO)$/salogl.obj	\
            $(SLO)$/winlayout.obj	\
            $(SLO)$/wntgdi.obj      \
            $(SLO)$/salnativewidgets-luna.obj
            

EXCEPTIONSFILES=	$(SLO)$/salprn.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
