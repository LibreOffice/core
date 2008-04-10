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
# $Revision: 1.13 $
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
PRJNAME=svx
TARGET=customshapes
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Types -------------------------------------

# Disable optimization for SunCC SPARC and MACOSX (funny loops
# when parsing e.g. "x+width/2"),
# also http://gcc.gnu.org/PR22392
.IF ("$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC") || "$(OS)"=="MACOSX" || ("$(OS)"=="LINUX" && "$(CPU)"=="P") 
NOOPTFILES= $(SLO)$/EnhancedCustomShapeFunctionParser.obj
.ENDIF

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/EnhancedCustomShapeEngine.obj			\
            $(SLO)$/EnhancedCustomShapeTypeNames.obj		\
            $(SLO)$/EnhancedCustomShapeGeometry.obj			\
            $(SLO)$/EnhancedCustomShape2d.obj				\
            $(SLO)$/EnhancedCustomShape3d.obj				\
            $(SLO)$/EnhancedCustomShapeFontWork.obj			\
            $(SLO)$/EnhancedCustomShapeHandle.obj			\
            $(SLO)$/EnhancedCustomShapeFunctionParser.obj	\
            $(SLO)$/tbxcustomshapes.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
