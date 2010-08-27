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

PRJNAME=sw
TARGET=ui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(GUI)" != "OS2"
.IF "$(GUI)" != "WNT"
.IF "$(COM)" != "BLC"
.IF "$(COM)" != "WTC"
LIBFLAGS=/NOI /NOE /PAGE:256
.ENDIF
.ENDIF
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

# andere Label als Verzeichnisnamen fuer VC++/NT

SUBLIBS1= \
        $(SLB)$/app.lib      \
        $(SLB)$/cctrl.lib    \
        $(SLB)$/chrdlg.lib   \
        $(SLB)$/config.lib   \
        $(SLB)$/dbui.lib     \
        $(SLB)$/dialog.lib   \
        $(SLB)$/dochdl.lib   \
                $(SLB)$/docvw.lib    \
                $(SLB)$/envelp.lib   \
        $(SLB)$/fldui.lib    \
        $(SLB)$/frmdlg.lib   \
        $(SLB)$/globdoc.lib

SUBLIBS2= \
        $(SLB)$/index.lib    \
        $(SLB)$/lingu.lib    \
        $(SLB)$/misc.lib     \
        $(SLB)$/ribbar.lib   \
        $(SLB)$/shells.lib   \
                $(SLB)$/smartmenu.lib \
                $(SLB)$/table.lib    \
        $(SLB)$/unoidl.lib   \
        $(SLB)$/utlui.lib    \
        $(SLB)$/web.lib   \
        $(SLB)$/uiview.lib   \
        $(SLB)$/wrtsh.lib

# -----------------------------------------------------------

################################################################

LIB1TARGET=$(SLB)$/ui1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/ui2.lib
LIB2FILES= \
        $(SUBLIBS2)

.INCLUDE :  target.mk

################################################################

kill:
    @$(RM) $(SLB)$/ui.lst
    @$(RM) $(SLB)$/ui.lib


