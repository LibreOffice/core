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
# $Revision: 1.8 $
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

PRJNAME=bridges
TARGET=bridges_remote_static
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

SLOFILES= 	\
        $(SLO)$/proxy.obj \
        $(SLO)$/stub.obj \
        $(SLO)$/remote.obj \
        $(SLO)$/mapping.obj \
        $(SLO)$/helper.obj \
        $(SLO)$/remote_types.obj

# Forte6 update 1 on Solaris Intel dies with internal compiler error
# on stub.cxx if optimization is on. Switch it off for now.
# To be reevaluated on compiler upgrade
.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES=\
        $(SLO)$/stub.obj
.ENDIF        

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


