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
TARGET=ui


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
    sc.src		 \
    pseudo.src	 \
    popup.src	 \
    toolbox.src	 \
    scstring.src \
    attrdlg.src  \
    sortdlg.src  \
    filter.src	 \
    namedlg.src  \
    dbnamdlg.src \
    subtdlg.src  \
    miscdlgs.src \
    datafdlg.src \
    autofmt.src  \
    solvrdlg.src \
    optsolver.src \
    solveroptions.src \
    tabopdlg.src \
    hdrcont.src  \
    globstr.src  \
    optdlg.src   \
    scerrors.src \
    textdlgs.src \
    scfuncs.src	 \
    crnrdlg.src	 \
    condfrmt.src \
    opredlin.src \
    simpref.src

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/ui.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

