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

PRJ=..

PRJNAME=starmath
TARGET=starmath

LIBTARGET=NO

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SMDLL=TRUE

SRS2NAME =smres
SRC2FILES = smres.src    \
            symbol.src   \
            toolbox.src \
            commands.src

SLO1FILES = \
        $(SLO)$/accessibility.obj \
        $(SLO)$/caret.obj         \
        $(SLO)$/cursor.obj        \
        $(SLO)$/edit.obj          \
        $(SLO)$/eqnolefilehdr.obj \
        $(SLO)$/mathmlexport.obj  \
        $(SLO)$/mathmlimport.obj  \
        $(SLO)$/symbol.obj        \
        $(SLO)$/types.obj         \
        $(SLO)$/unodoc.obj        \
        $(SLO)$/action.obj        \
        $(SLO)$/cfgitem.obj       \
        $(SLO)$/config.obj        \
        $(SLO)$/dialog.obj        \
        $(SLO)$/document.obj      \
        $(SLO)$/format.obj        \
        $(SLO)$/mathtype.obj \
        $(SLO)$/node.obj \
        $(SLO)$/parse.obj \
        $(SLO)$/register.obj      \
        $(SLO)$/smdll.obj         \
        $(SLO)$/toolbox.obj       \
        $(SLO)$/typemap.obj       \
        $(SLO)$/smmod.obj         \
        $(SLO)$/utility.obj \
        $(SLO)$/rect.obj          \
        $(SLO)$/unomodel.obj      \
        $(SLO)$/view.obj          \
        $(SLO)$/visitors.obj

SLO2FILES = \
        $(SLO)$/register.obj \
        $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO)$/smdetect.obj \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"$(DLLPRE)sm$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"$(DLLPRE)sm$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SRS)$/smres.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


$(SRS)$/$(SRS2NAME).srs  : $(LOCALIZE_ME_DEST)

