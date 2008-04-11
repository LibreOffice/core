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
# $Revision: 1.7 $
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

PRJ=..$/

PRJNAME=basebmp
TARGET=wrapper
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk


# TODO(F3) unify BASEBMP_NO_NESTED_TEMPLATE_PARAMETER and
# VIGRA_WITHOUT_NESTED_TEMPLATES_PARAMS
#
# - VIGRA_WITHOUT_NESTED_TEMPLATES_PARAMS avoids template parameters
#   to templates - the template keyword is broken for SunCC
# - VIGRA_HAS_LONG_LONG enables long long overloads (but appears to not work
#   reliably in some environments, and is thus not enabled)
# - VIGRA_HAS_LONG_DOUBLE enables long double overloads 

.IF "$(OS)"!="WNT"
CDEFS+= -DVIGRA_HAS_LONG_DOUBLE
.ENDIF

.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
CDEFS+= -DBASEBMP_NO_NESTED_TEMPLATE_PARAMETER -DVIGRA_WITHOUT_NESTED_TEMPLATE_PARAMS
.ENDIF

# --- Common ----------------------------------------------------------

SLOFILES =	\
    $(SLO)$/bitmapdevice.obj \
    $(SLO)$/debug.obj \
    $(SLO)$/polypolygonrenderer.obj

#	$(SLO)$/genericintegerimageaccessor.obj \

# ==========================================================================

.INCLUDE :	target.mk
