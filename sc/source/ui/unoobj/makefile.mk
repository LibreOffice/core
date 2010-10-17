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

PRJNAME=sc
TARGET=unoobj

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
SLO1FILES =  \
        $(SLO)$/docuno.obj \
        $(SLO)$/servuno.obj \
        $(SLO)$/defltuno.obj \
        $(SLO)$/drdefuno.obj \
        $(SLO)$/cellsuno.obj \
        $(SLO)$/tokenuno.obj \
        $(SLO)$/textuno.obj \
        $(SLO)$/notesuno.obj \
        $(SLO)$/cursuno.obj \
        $(SLO)$/srchuno.obj \
        $(SLO)$/fielduno.obj \
        $(SLO)$/miscuno.obj \
        $(SLO)$/optuno.obj \
        $(SLO)$/appluno.obj \
        $(SLO)$/funcuno.obj \
        $(SLO)$/nameuno.obj \
        $(SLO)$/viewuno.obj \
        $(SLO)$/dispuno.obj \
        $(SLO)$/datauno.obj \
        $(SLO)$/dapiuno.obj \
        $(SLO)$/chartuno.obj \
        $(SLO)$/chart2uno.obj \
        $(SLO)$/shapeuno.obj \
        $(SLO)$/pageuno.obj \
        $(SLO)$/forbiuno.obj \
        $(SLO)$/styleuno.obj \
        $(SLO)$/afmtuno.obj \
        $(SLO)$/fmtuno.obj \
        $(SLO)$/linkuno.obj \
        $(SLO)$/targuno.obj \
        $(SLO)$/convuno.obj \
        $(SLO)$/editsrc.obj \
        $(SLO)$/unoguard.obj \
        $(SLO)$/confuno.obj \
        $(SLO)$/filtuno.obj \
        $(SLO)$/unodoc.obj \
        $(SLO)$/addruno.obj \
        $(SLO)$/eventuno.obj \
        $(SLO)$/listenercalls.obj \
        $(SLO)$/cellvaluebinding.obj \
        $(SLO)$/celllistsource.obj \
        $(SLO)$/warnpassword.obj \
        $(SLO)$/unoreflist.obj \
        $(SLO)$/ChartRangeSelectionListener.obj

SLO2FILES = \
    $(SLO)$/scdetect.obj								\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
             $(SLO)$/cellsuno.obj
.ENDIF

# Work around bug in gcc 4.2 / 4.3, see
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=35182
.IF "$(COM)"=="GCC"
NOOPTFILES+= \
    $(SLO)$/chart2uno.obj
.ENDIF


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

