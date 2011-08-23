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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=sw_ui

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

#.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/inc$/sw.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw

.IF "$(GUI)" != "OS2"
.IF "$(GUI)" != "WNT"
.IF "$(GUI)" != "MAC"
.IF "$(COM)" != "BLC"
.IF "$(COM)" != "WTC"
LIBFLAGS=/NOI /NOE /PAGE:256
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF

################################################################

# andere Label als Verzeichnisnamen fuer VC++/NT

LIB1TARGET=$(SLB)$/sw_ui.lib
LIB1FILES= \
        $(SLB)$/sw_app.lib      \
        $(SLB)$/sw_config.lib   \
        $(SLB)$/sw_dbui.lib     \
        $(SLB)$/sw_dochdl.lib   \
        $(SLB)$/sw_utlui.lib    \
        $(SLB)$/sw_fldui.lib    \
        $(SLB)$/sw_globdoc.lib  \
        $(SLB)$/sw_index.lib    \
        $(SLB)$/sw_unoidl.lib   \
        $(SLB)$/sw_web.lib      \
        $(SLB)$/sw_wrtsh.lib

.INCLUDE :  target.mk

################################################################


kill:
    @$(RM) $(SLB)$/ui.lst
    @$(RM) $(SLB)$/ui.lib

