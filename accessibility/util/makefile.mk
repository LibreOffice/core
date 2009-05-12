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
# $Revision: 1.5 $
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

PRJ=..
PRJNAME=accessibility
TARGET=acc
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

#.INCLUDE :	svpre.mk
#.INCLUDE :	settings.mk
#.INCLUDE :	sv.mk

LDUMP=ldump2.exe

# --- Library -----------------------------------
# --- acc ---------------------------------------
LIB1TARGET=$(SLB)$/$(PRJNAME).lib
LIB1FILES=\
        $(SLB)$/standard.lib \
        $(SLB)$/extended.lib \
        $(SLB)$/helper.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(VCLLIB)           \
        $(COMPHELPERLIB)    \
        $(SOTLIB)           \
        $(CPPULIB)          \
        $(CPPUHELPERLIB)    \
        $(UNOTOOLSLIB)      \
        $(TKLIB)            \
        $(TOOLSLIB)         \
        $(SVTOOLLIB)        \
        $(SVLLIB)           \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEPN=$(LIB1TARGET)	\
        makefile.mk


SHL1VERSIONMAP= $(TARGET).map 
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

# === .res file ==========================================================

RES1FILELIST=\
            $(SRS)$/helper.srs

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

