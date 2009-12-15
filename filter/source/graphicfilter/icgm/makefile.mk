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
# $Revision: 1.12 $
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
PRJNAME=goodies
TARGET=icgm
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------
.IF "$(L10N_framework)"==""
SLOFILES =	$(SLO)$/cgm.obj		\
            $(SLO)$/chart.obj	\
            $(SLO)$/class0.obj	\
            $(SLO)$/class1.obj	\
            $(SLO)$/class2.obj	\
            $(SLO)$/class3.obj	\
            $(SLO)$/class4.obj	\
            $(SLO)$/class5.obj	\
            $(SLO)$/class7.obj	\
            $(SLO)$/classx.obj	\
            $(SLO)$/outact.obj	\
            $(SLO)$/actimpr.obj	\
            $(SLO)$/bundles.obj	\
            $(SLO)$/bitmap.obj	\
            $(SLO)$/elements.obj
#			$(SLO)$/svdem.obj

SHL1TARGET	=	icg$(DLLPOSTFIX)
SHL1IMPLIB	=	icgm
SHL1VERSIONMAP=exports.map
SHL1DEF		=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS	=	$(SLB)$/icgm.lib

DEF1NAME=$(SHL1TARGET)

.IF "$(GUI)"=="OS2"
SHL1OBJS	=	$(SLO)$/class0.obj
.ENDIF

SHL1STDLIBS = \
            $(TKLIB)		\
            $(VCLLIB)		\
            $(UNOTOOLSLIB)	\
            $(TOOLSLIB)		\
            $(CPPULIB)		\
            $(SALLIB)
.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

