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

PRJ=..$/..

PRJNAME=basic
TARGET=runtime

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk


# --- Allgemein -----------------------------------------------------------

SLOFILES=	\
    $(SLO)$/basrdll.obj	\
    $(SLO)$/inputbox.obj	\
    $(SLO)$/runtime.obj	\
    $(SLO)$/step0.obj	\
    $(SLO)$/step1.obj	\
    $(SLO)$/step2.obj	\
    $(SLO)$/iosys.obj	\
    $(SLO)$/stdobj.obj	\
    $(SLO)$/stdobj1.obj	\
    $(SLO)$/methods.obj	\
    $(SLO)$/methods1.obj	\
    $(SLO)$/props.obj	\
    $(SLO)$/ddectrl.obj	\
    $(SLO)$/dllmgr.obj

.IF "$(GUI)$(CPU)" == "WINI"
SLOFILES+=	$(SLO)$/win.obj
.ENDIF

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
SLOFILES+=	$(SLO)$/wnt.obj
.ENDIF

.IF "$(GUI)$(COM)$(CPU)" == "WNTGCCI"
SLOFILES+=	$(SLO)$/wnt-mingw.obj
.ENDIF

.IF "$(GUI)$(CPU)" == "OS2I"
#FIXME SLOFILES+= $(SLO)$/os2.obj
.ENDIF

EXCEPTIONSFILES=$(SLO)$/step0.obj	\
        $(SLO)$/step2.obj	\
        $(SLO)$/methods.obj	\
        $(SLO)$/methods1.obj	\
        $(SLO)$/iosys.obj	\
        $(SLO)$/runtime.obj

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

$(SLO)$/%.obj: %.s
#kendy: Cut'n'paste from bridges/source/cpp_uno/mingw_intel/makefile.mk
#cmc: Ideally --noexecstack would be in operations, but with #i51385# pyuno
#remote bridgeing breaks
#    $(CC) -Wa,--noexecstack -c -o $(SLO)$/$(@:b).o $<
    $(CC) -c -o $(SLO)$/$(@:b).obj $<
    touch $@
