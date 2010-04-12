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

PRJNAME=sd
TARGET=sdslots
TARGET2=sdgslots
SDI1EXPORT=sdraw
SDI2EXPORT=sdraw
SVSDIINC=$(PRJ)$/source$/ui$/inc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

# --- Files --------------------------------------------------------


SDI1NAME=$(TARGET)
SDI2NAME=$(TARGET2)
SIDHRCNAME=SDSLOTS.HRC

SVSDI1DEPEND= \
        $(SOLARINCXDIR)$/sfx2/sfx.sdi \
        $(SOLARINCXDIR)$/sfx2/sfxitems.sdi \
        $(SOLARINCXDIR)$/svx/svx.sdi \
        $(SOLARINCXDIR)$/svx/svxitems.sdi \
        $(SOLARINCXDIR)$/svx/xoitems.sdi \
    sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    docshell.sdi \
    drtxtob.sdi \
        drbezob.sdi \
        drgrfob.sdi \
    _drvwsh.sdi \
    drviewsh.sdi \
        mediaob.sdi \
        tables.sdi\
    outlnvsh.sdi \
    SlideSorterController.sdi \
    ViewShellBase.sdi \
    ToolPanelViewShell.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

SVSDI2DEPEND= \
        $(SOLARINCXDIR)$/sfx2/sfx.sdi \
        $(SOLARINCXDIR)$/sfx2/sfxitems.sdi \
        $(SOLARINCXDIR)$/svx/svx.sdi \
        $(SOLARINCXDIR)$/svx/svxitems.sdi \
        $(SOLARINCXDIR)$/svx/xoitems.sdi \
        sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    grdocsh.sdi \
    drtxtob.sdi \
        drbezob.sdi \
        drgrfob.sdi \
    _drvwsh.sdi \
    grviewsh.sdi \
        mediaob.sdi \
    outlnvsh.sdi \
    ViewShellBase.sdi \
    ToolPanelViewShell.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

# --- Targets -------------------------------------------------------
.ENDIF
.INCLUDE :  target.mk
