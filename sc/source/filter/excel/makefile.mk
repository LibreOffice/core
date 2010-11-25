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
TARGET=excel

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(EXCEPTIONSFILES)

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES = \
        $(SLO)$/xiescher.obj
.ENDIF

EXCEPTIONSFILES = \
        $(SLO)$/colrowst.obj				\
        $(SLO)$/excdoc.obj				\
        $(SLO)$/excel.obj				\
        $(SLO)$/excform.obj				\
        $(SLO)$/excform8.obj				\
        $(SLO)$/excimp8.obj				\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/exctools.obj				\
        $(SLO)$/expop2.obj				\
        $(SLO)$/fontbuff.obj				\
        $(SLO)$/frmbase.obj				\
        $(SLO)$/impop.obj				\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/read.obj				\
        $(SLO)$/tokstack.obj				\
        $(SLO)$/xechart.obj				\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xeescher.obj				\
        $(SLO)$/xeformula.obj				\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj				\
        $(SLO)$/xename.obj				\
        $(SLO)$/xepage.obj				\
        $(SLO)$/xepivot.obj				\
        $(SLO)$/xerecord.obj				\
        $(SLO)$/xeroot.obj				\
        $(SLO)$/xestream.obj				\
        $(SLO)$/xestring.obj				\
        $(SLO)$/xestyle.obj				\
        $(SLO)$/xetable.obj				\
        $(SLO)$/xeview.obj				\
        $(SLO)$/xichart.obj				\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xiformula.obj				\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj				\
        $(SLO)$/xiname.obj				\
        $(SLO)$/xipage.obj				\
        $(SLO)$/xipivot.obj				\
        $(SLO)$/xiroot.obj				\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xistring.obj				\
        $(SLO)$/xistyle.obj				\
        $(SLO)$/xiview.obj				\
        $(SLO)$/xladdress.obj				\
        $(SLO)$/xlchart.obj				\
        $(SLO)$/xlescher.obj				\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlpage.obj				\
        $(SLO)$/xlpivot.obj				\
        $(SLO)$/xlroot.obj				\
        $(SLO)$/xlstyle.obj				\
        $(SLO)$/xltoolbar.obj				\
        $(SLO)$/xltools.obj				\
        $(SLO)$/xltracer.obj				\
        $(SLO)$/xlview.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

