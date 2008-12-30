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
# $Revision: 1.27 $
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

PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=xml
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/makefile.pmk

.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
CFLAGSCXX+=-instances=static
.ENDIF

# --- Files ---


SLOFILES=\
    $(SLO)$/matchlocale.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/simpletypehelper.obj \
    $(SLO)$/valueconverter.obj \
    $(SLO)$/elementparser.obj \
    $(SLO)$/elementformatter.obj \
    $(SLO)$/basicparser.obj \
    $(SLO)$/layerparser.obj \
    $(SLO)$/schemaparser.obj \
    $(SLO)$/parsersvc.obj \
    $(SLO)$/writersvc.obj \
    $(SLO)$/layerwriter.obj \
    $(SLO)$/valueformatter.obj \
    $(SLO)$/xmlstrings.obj \

# --- Targets ---

.INCLUDE : target.mk

