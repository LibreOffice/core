#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: ihi $ $Date: 2006-11-14 13:19:52 $
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

PRJNAME=svx
TARGET=engine3d
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
        $(SLO)$/obj3d.obj 		\
        $(SLO)$/pntobj3d.obj     \
        $(SLO)$/label3d.obj		\
        $(SLO)$/scene3d.obj		\
        $(SLO)$/polysc3d.obj		\
        $(SLO)$/cube3d.obj		\
        $(SLO)$/sphere3d.obj		\
        $(SLO)$/extrud3d.obj     \
        $(SLO)$/lathe3d.obj		\
        $(SLO)$/polygn3d.obj 		\
        $(SLO)$/svx3ditems.obj	\
        $(SLO)$/deflt3d.obj		\
        $(SLO)$/e3dundo.obj      \
        $(SLO)$/volume3d.obj		\
        $(SLO)$/viewpt3d.obj		\
        $(SLO)$/viewpt3d2.obj		\
        $(SLO)$/camera3d.obj		\
        $(SLO)$/objfac3d.obj 	\
        $(SLO)$/dragmt3d.obj		\
        $(SLO)$/view3d.obj		\
        $(SLO)$/view3d1.obj       \
        $(SLO)$/float3d.obj

#disable optimizer for MSCompiler and ICC
.IF "$(COM)"=="ICC" || "$(COM)"=="MSC"
NOOPTFILES=\
        $(SLO)$/viewpt3d.obj
.ENDIF

SRS1NAME=engine3d
SRC1FILES=\
        string3d.src	\
        float3d.src

.INCLUDE :  target.mk
