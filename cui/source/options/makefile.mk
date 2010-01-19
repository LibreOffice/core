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
# $Revision: 1.65 $
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
PRJNAME=cui
TARGET=options
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=options
SRC1FILES =  \
        connpooloptions.src \
        dbregister.src \
        doclinkdialog.src \
        fontsubs.src \
        internationaloptions.src \
        optaccessibility.src \
        optasian.src \
        optchart.src \
        optcolor.src \
        optctl.src \
        optdict.src \
        optfltr.src \
        optgdlg.src \
        optgenrl.src \
        opthtml.src \
        optimprove.src \
        optinet2.src \
        optjava.src \
        optjsearch.src \
        optlingu.src \
        optmemory.src \
        optpath.src \
        optsave.src \
        optupdt.src \
        readonlyimage.src \
        securityoptions.src \
        treeopt.src \
        webconninfo.src \


SLOFILES+=\
        $(SLO)$/cfgchart.obj \
        $(SLO)$/connpoolconfig.obj \
        $(SLO)$/connpooloptions.obj \
        $(SLO)$/connpoolsettings.obj \
        $(SLO)$/cuisrchdlg.obj \
        $(SLO)$/dbregister.obj \
        $(SLO)$/dbregisterednamesconfig.obj \
        $(SLO)$/dbregistersettings.obj \
        $(SLO)$/doclinkdialog.obj \
        $(SLO)$/fontsubs.obj \
        $(SLO)$/internationaloptions.obj \
        $(SLO)$/optaccessibility.obj \
        $(SLO)$/optasian.obj \
        $(SLO)$/optchart.obj \
        $(SLO)$/optcolor.obj \
        $(SLO)$/optctl.obj \
        $(SLO)$/optdict.obj \
        $(SLO)$/optfltr.obj \
        $(SLO)$/optgdlg.obj \
        $(SLO)$/optgenrl.obj \
        $(SLO)$/optHeaderTabListbox.obj \
        $(SLO)$/opthtml.obj \
        $(SLO)$/optimprove.obj \
        $(SLO)$/optimprove2.obj \
        $(SLO)$/optinet2.obj \
        $(SLO)$/optjava.obj \
        $(SLO)$/optjsearch.obj \
        $(SLO)$/optlingu.obj \
        $(SLO)$/optmemory.obj \
        $(SLO)$/optpath.obj \
        $(SLO)$/optsave.obj \
        $(SLO)$/optupdt.obj \
        $(SLO)$/radiobtnbox.obj \
        $(SLO)$/readonlyimage.obj \
        $(SLO)$/sdbcdriverenum.obj \
        $(SLO)$/securityoptions.obj \
        $(SLO)$/treeopt.obj \
        $(SLO)$/webconninfo.obj \


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
