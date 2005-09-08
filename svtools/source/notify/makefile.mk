#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 16:32:26 $
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

PRJNAME=svtools
TARGET=notify

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svl.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/smplhint.obj \
        $(SLO)$/hint.obj \
        $(SLO)$/lstner.obj \
        $(SLO)$/isethint.obj \
        $(SLO)$/cancel.obj \
        $(SLO)$/brdcst.obj \
        $(SLO)$/listener.obj \
        $(SLO)$/listenerbase.obj \
        $(SLO)$/listeneriter.obj \
        $(SLO)$/broadcast.obj

HXX1TARGET= notify
HXX1EXT=    hxx
HXX1FILES=  $(INC)$/hint.hxx                \
             $(INC)$/smplhint.hxx            \
             $(INC)$/lstner.hxx              \
             $(INC)$/brdcst.hxx
HXX1EXCL=   -E:*include*

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

