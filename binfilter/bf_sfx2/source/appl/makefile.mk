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

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=sfx2_appl

NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

INC+= -I$(PRJ)$/inc$/bf_sfx2

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
CFLAGS+=-Od
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/sfx2_appuno.obj \
    $(SLO)$/sfx2_appmain.obj \
    $(SLO)$/sfx2_appopen.obj \
    $(SLO)$/sfx2_appinit.obj \
    $(SLO)$/sfx2_appmisc.obj \
    $(SLO)$/sfx2_appcfg.obj \
    $(SLO)$/sfx2_appquit.obj \
    $(SLO)$/sfx2_appdata.obj \
    $(SLO)$/sfx2_app.obj \
    $(SLO)$/sfx2_appbas.obj \
    $(SLO)$/sfx2_appdde.obj \
    $(SLO)$/sfx2_sfxdll.obj \
    $(SLO)$/sfx2_module.obj \
    $(SLO)$/sfx2_dlgcont.obj \
    $(SLO)$/sfx2_namecont.obj \
    $(SLO)$/sfx2_scriptcont.obj \
    $(SLO)$/sfx2_imestatuswindow.obj 

EXCEPTIONSFILES=\
    $(SLO)$/sfx2_appopen.obj \
    $(SLO)$/sfx2_appmain.obj			\
    $(SLO)$/sfx2_appmisc.obj			\
    $(SLO)$/sfx2_appinit.obj			\
    $(SLO)$/sfx2_appcfg.obj			

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

