#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 04:06:40 $
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

PRJ=..

PRJNAME=vos
TARGET=cppvos
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=       $(SLO)$/conditn.obj     \
                $(SLO)$/mutex.obj       \
                $(SLO)$/object.obj      \
                $(SLO)$/refernce.obj    \
                $(SLO)$/socket.obj      \
                $(SLO)$/thread.obj      \
                $(SLO)$/stream.obj      \
                $(SLO)$/module.obj      \
                $(SLO)$/timer.obj       \
                $(SLO)$/process.obj     \
                $(SLO)$/security.obj    \
                $(SLO)$/signal.obj      \
                $(SLO)$/pipe.obj        \
                $(SLO)$/xception.obj


.IF "$(UPDATER)"=="YES"
OBJFILES=       $(OBJ)$/conditn.obj     \
                $(OBJ)$/mutex.obj       \
                $(OBJ)$/object.obj      \
                $(OBJ)$/refernce.obj    \
                $(OBJ)$/socket.obj      \
                $(OBJ)$/thread.obj      \
                $(OBJ)$/stream.obj      \
                $(OBJ)$/module.obj      \
                $(OBJ)$/timer.obj       \
                $(OBJ)$/process.obj     \
                $(OBJ)$/security.obj    \
                $(OBJ)$/signal.obj      \
                $(OBJ)$/pipe.obj        \
                $(OBJ)$/xception.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



