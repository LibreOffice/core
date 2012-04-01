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
PRJNAME=vbahelper
TARGET=msforms

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DVBA_OOBUILD_HACK

SLOFILES=\
    $(SLO)$/vbacontrol.obj \
    $(SLO)$/vbacontrols.obj \
    $(SLO)$/vbabutton.obj \
    $(SLO)$/vbacombobox.obj \
    $(SLO)$/vbalabel.obj \
    $(SLO)$/vbatextbox.obj \
    $(SLO)$/vbaradiobutton.obj \
    $(SLO)$/vbalistbox.obj \
    $(SLO)$/vbatogglebutton.obj \
    $(SLO)$/vbacheckbox.obj \
    $(SLO)$/vbaframe.obj \
    $(SLO)$/vbascrollbar.obj \
    $(SLO)$/vbaprogressbar.obj \
    $(SLO)$/vbamultipage.obj \
    $(SLO)$/vbalistcontrolhelper.obj \
    $(SLO)$/vbaspinbutton.obj \
    $(SLO)$/vbasystemaxcontrol.obj \
    $(SLO)$/vbaimage.obj \
    $(SLO)$/vbapages.obj \
    $(SLO)$/vbauserform.obj \
    $(SLO)$/service.obj \

# #FIXME vbapropvalue needs to move to vbahelper

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

