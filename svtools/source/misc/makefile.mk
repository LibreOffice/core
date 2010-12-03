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

PRJ=..$/..
PRJNAME=svtools
TARGET=misc

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

#use local "bmp" as it may not yet be delivered

SRS1NAME=misc
SRC1FILES=\
    ehdl.src \
    helpagent.src \
    imagemgr.src      \
    langtab.src 

SRS2NAME=ehdl
SRC2FILES=\
    errtxt.src

SLOFILES=\
    $(SLO)$/acceleratorexecute.obj  \
    $(SLO)$/chartprettypainter.obj \
    $(SLO)$/cliplistener.obj        \
    $(SLO)$/dialogclosedlistener.obj\
    $(SLO)$/dialogcontrolling.obj   \
    $(SLO)$/ehdl.obj                \
    $(SLO)$/embedhlp.obj            \
    $(SLO)$/embedtransfer.obj       \
    $(SLO)$/filechangedchecker.obj  \
    $(SLO)$/helpagentwindow.obj     \
    $(SLO)$/imagemgr.obj            \
    $(SLO)$/imageresourceaccess.obj \
    $(SLO)$/imap.obj                \
    $(SLO)$/imap2.obj               \
    $(SLO)$/imap3.obj               \
    $(SLO)$/itemdel.obj				\
    $(SLO)$/langtab.obj				\
    $(SLO)$/langhelp.obj			\
    $(SLO)$/stringtransfer.obj      \
    $(SLO)$/svtaccessiblefactory.obj \
    $(SLO)$/svtdata.obj             \
    $(SLO)$/templatefoldercache.obj \
    $(SLO)$/transfer.obj            \
    $(SLO)$/transfer2.obj           \
    $(SLO)$/unitconv.obj           \
    $(SLO)$/bindablecontrolhelper.obj   \
    $(SLO)$/filterutils.obj   \
    $(SLO)$/wallitem.obj           \
    $(SLO)$/xwindowitem.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

