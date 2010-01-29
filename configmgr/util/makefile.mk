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
# $Revision: 1.25 $
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
PRJNAME=configmgr
TARGET=configmgr

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk
.IF "$(L10N_framework)"==""
.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk
DLLPRE = 

# --- Library -----------------------------------

SHL1TARGET=	$(CFGMGR_TARGET)$(CFGMGR_MAJOR).uno
SHL1VERSIONMAP= $(TARGET).map

SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(COMPHELPERLIB)			\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALHELPERLIB)				\
    $(SALLIB)                   \
    $(TOOLSLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/registry.lib	\
            $(SLB)$/treecache.lib	\
            $(SLB)$/misc.lib		\
            $(SLB)$/backend.lib		\
            $(SLB)$/localbe.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/treemgr.lib		\
            $(SLB)$/api2.lib		\
            $(SLB)$/api.lib			\
            $(SLB)$/data.lib		\
            $(SLB)$/cm.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------
.ENDIF # L10N_framework

.INCLUDE : target.mk

