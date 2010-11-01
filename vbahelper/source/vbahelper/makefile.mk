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

PRJ=..$/../
PRJNAME=vbahelper
TARGET=vbahelperbits

ENABLE_EXCEPTIONS := TRUE
VISIBILITY_HIDDEN=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

SLOFILES=\
    $(SLO)$/vbahelper.obj \
    $(SLO)$/vbapropvalue.obj \
    $(SLO)$/vbacommandbars.obj \
    $(SLO)$/vbacommandbar.obj \
    $(SLO)$/vbacommandbarcontrols.obj \
    $(SLO)$/vbacommandbarcontrol.obj \
    $(SLO)$/vbacommandbarhelper.obj \
    $(SLO)$/vbaapplicationbase.obj \
    $(SLO)$/vbawindowbase.obj \
    $(SLO)$/vbadocumentbase.obj \
    $(SLO)$/vbadocumentsbase.obj \
    $(SLO)$/vbaglobalbase.obj \
    $(SLO)$/vbafontbase.obj \
    $(SLO)$/vbadialogbase.obj \
    $(SLO)$/vbadialogsbase.obj \
    $(SLO)$/vbashape.obj \
    $(SLO)$/vbacolorformat.obj \
    $(SLO)$/vbashapes.obj \
    $(SLO)$/vbalineformat.obj \
    $(SLO)$/vbafillformat.obj \
    $(SLO)$/vbapictureformat.obj \
    $(SLO)$/vbashaperange.obj \
    $(SLO)$/vbatextframe.obj \
    $(SLO)$/vbapagesetupbase.obj \
    $(SLO)$/vbaeventshelperbase.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

