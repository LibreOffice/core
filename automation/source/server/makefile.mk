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
# $Revision: 1.6 $
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

PRJNAME=automation
TARGET=server

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/XMLParser.obj \
    $(SLO)$/recorder.obj \
    $(SLO)$/svcommstream.obj \
    $(SLO)$/cmdbasestream.obj \
    $(SLO)$/scmdstrm.obj \
    $(SLO)$/statemnt.obj \
    $(SLO)$/sta_list.obj \
    $(SLO)$/editwin.obj \
    $(SLO)$/server.obj \
    $(SLO)$/retstrm.obj \
    $(SLO)$/profiler.obj


OBJFILES = \
    $(OBJ)$/cmdbasestream.obj \
    $(OBJ)$/svcommstream.obj

EXCEPTIONSFILES = \
    $(SLO)$/server.obj \
    $(SLO)$/XMLParser.obj

NOOPTFILES = $(SLO)$/statemnt.obj

.IF "$(OS)"=="SOLARIS"
SLOFILES +=   $(SLO)$/prof_usl.obj 
.ELSE
SLOFILES +=   $(SLO)$/prof_nul.obj 
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
