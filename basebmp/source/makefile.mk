#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2006-10-12 13:47:55 $
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
