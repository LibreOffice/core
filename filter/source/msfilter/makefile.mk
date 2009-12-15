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
# $Revision: 1.17 $
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

PRJ=../..

PRJNAME=filter
TARGET=msfilter
ENABLE_EXCEPTIONS=true
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/countryid.obj	\
    $(SLO)$/escherex.obj	\
    $(SLO)$/eschesdo.obj    \
    $(SLO)$/msdffimp.obj	\
    $(SLO)$/msoleexp.obj	\
    $(SLO)$/msvbasic.obj	\
    $(SLO)$/svxmsbas.obj	\
    $(SLO)$/msocximex.obj	\
    $(SLO)$/mscodec.obj		\
    $(SLO)$/msfiltertracer.obj \
    $(SLO)$/svdfppt.obj		\
    $(SLO)$/svxmsbas2.obj

SHL1TARGET= msfilter$(DLLPOSTFIX)
SHL1IMPLIB=	i$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1USE_EXPORTS=name
SHL1STDLIBS= \
             $(VBAHELPERLIB) \
             $(SVXCORELIB) \
             $(SFX2LIB) \
             $(XMLOFFLIB) \
             $(BASEGFXLIB) \
             $(BASICLIB) \
             $(SVTOOLLIB) \
             $(TKLIB) \
             $(VCLLIB) \
             $(SVLLIB) \
             $(SOTLIB) \
             $(UNOTOOLSLIB) \
             $(TOOLSLIB) \
             $(XMLSCRIPTLIB) \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(CPPULIB) \
             $(SALLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

.INCLUDE :  target.mk


