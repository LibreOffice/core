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

PRJ=..$/..$/..
PRJNAME=binfilter
TARGET=misc

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_svtools

# --- Files --------------------------------------------------------

SRS1NAME=ehdl
SRC1FILES=\
    ctrltool.src\
    errtxt.src

SLOFILES=\
    $(SLO)$/svt_asynclink.obj			\
    $(SLO)$/svt_ctrltool.obj			\
    $(SLO)$/svt_imageresourceaccess.obj \
    $(SLO)$/svt_templatefoldercache.obj \
    $(SLO)$/svt_urihelper.obj           \
    $(SLO)$/svt_ehdl.obj                \
    $(SLO)$/svt_flbytes.obj             \
    $(SLO)$/svt_imap.obj                \
    $(SLO)$/svt_imap2.obj               \
    $(SLO)$/svt_imap3.obj               \
    $(SLO)$/svt_ownlist.obj             \
    $(SLO)$/svt_vcldata.obj             \
    $(SLO)$/svt_restrictedpaths.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

