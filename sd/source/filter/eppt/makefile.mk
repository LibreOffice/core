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
# $Revision: 1.15 $
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
PRJNAME=sd
TARGET=eppt
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/epptso.obj
.ENDIF

SLOFILES =	$(SLO)$/eppt.obj				\
            $(SLO)$/epptso.obj				\
            $(SLO)$/escherex.obj			\
            $(SLO)$/pptexanimations.obj		\
            $(SLO)$/pptexsoundcollection.obj

SHL1TARGET	=	emp$(DLLPOSTFIX)
SHL1IMPLIB	=	eppt
SHL1VERSIONMAP=exports.map
SHL1DEF 	=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS	=	$(SLB)$/eppt.lib

DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS = 	$(SVXLIB)			\
                $(SFX2LIB)          \
                $(SVTOOLLIB)		\
                $(SOTLIB)           \
                $(GOODIESLIB)		\
                $(VCLLIB)			\
                $(SOTLIB)			\
                $(UNOTOOLSLIB)		\
                $(TOOLSLIB) 		\
                $(UCBHELPERLIB)		\
                $(CPPULIB)			\
                $(SALLIB)			\
                $(COMPHELPERLIB)	\
                $(I18NISOLANGLIB)

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk
