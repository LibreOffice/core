#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME=binfilter
TARGET=sw_docnode

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw
.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF


# --- Files --------------------------------------------------------

CXXFILES = \
        sw_ndindex.cxx \
        sw_ndcopy.cxx \
        sw_ndnotxt.cxx \
        sw_ndnum.cxx \
        sw_ndsect.cxx \
        sw_ndtbl.cxx \
        sw_ndtbl1.cxx \
        sw_node.cxx \
        sw_node2lay.cxx \
        sw_nodes.cxx \
        sw_section.cxx \
        sw_swbaslnk.cxx



SLOFILES =	\
        $(SLO)$/sw_ndindex.obj \
        $(SLO)$/sw_ndcopy.obj \
        $(SLO)$/sw_ndnotxt.obj \
        $(SLO)$/sw_ndnum.obj \
        $(SLO)$/sw_ndsect.obj \
        $(SLO)$/sw_ndtbl.obj \
        $(SLO)$/sw_ndtbl1.obj \
        $(SLO)$/sw_node.obj \
        $(SLO)$/sw_node2lay.obj \
        $(SLO)$/sw_nodes.obj \
        $(SLO)$/sw_section.obj \
        $(SLO)$/sw_swbaslnk.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

