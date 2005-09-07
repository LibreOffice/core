#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:49:24 $
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
PRJNAME=basegfx
TARGET=polygon

#UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
#ENABLE_EXCEPTIONS=FALSE
#USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/b2dpolygon.obj						\
        $(SLO)$/b2dpolygontools.obj					\
        $(SLO)$/b2dpolypolygon.obj					\
        $(SLO)$/b2dpolypolygontools.obj				\
        $(SLO)$/b2dsvgpolypolygon.obj				\
        $(SLO)$/b2dlinegeometry.obj					\
        $(SLO)$/b2dpolypolygoncutter.obj			\
        $(SLO)$/b2dpolypolygonrasterconverter.obj	\
        $(SLO)$/b2dpolygontriangulator.obj			\
        $(SLO)$/b2dpolygoncutandtouch.obj			\
        $(SLO)$/b3dpolygon.obj						\
        $(SLO)$/b3dpolygontools.obj					\
        $(SLO)$/b3dpolypolygon.obj					\
        $(SLO)$/b3dpolypolygontools.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
