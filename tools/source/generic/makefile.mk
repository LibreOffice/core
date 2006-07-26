#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2006-07-26 08:19:09 $
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

PRJNAME=tools
TARGET=gen

# --- Settings -----------------------------------------------------

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

.IF "$(WITH_GPC)"!="NO"
CDEFS+=-DHAVE_GPC_H
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = $(SLO)$/poly.obj $(OBJ)$/poly.obj

SLOFILES=       $(SLO)$/toolsin.obj     \
            $(SLO)$/link.obj                \
            $(SLO)$/bigint.obj              \
            $(SLO)$/fract.obj               \
            $(SLO)$/color.obj               \
            $(SLO)$/gen.obj			\
            $(SLO)$/config.obj		\
            $(SLO)$/poly.obj		\
            $(SLO)$/poly2.obj		\
            $(SLO)$/svborder.obj \
            $(SLO)$/line.obj

OBJFILES=       $(OBJ)$/toolsin.obj     \
            $(OBJ)$/link.obj                \
            $(OBJ)$/bigint.obj              \
            $(OBJ)$/fract.obj               \
            $(OBJ)$/color.obj               \
            $(OBJ)$/gen.obj			\
            $(OBJ)$/config.obj		\
            $(OBJ)$/poly.obj		\
            $(OBJ)$/poly2.obj		\
            $(OBJ)$/svborder.obj \
            $(OBJ)$/line.obj

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
