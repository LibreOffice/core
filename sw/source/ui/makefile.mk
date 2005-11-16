#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: obo $ $Date: 2005-11-16 13:54:09 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sw
TARGET=ui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(GUI)" != "WNT"
.IF "$(GUI)" != "MAC"
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
    @+$(RM) $(SLB)$/ui.lst
    @+$(RM) $(SLB)$/ui.lib


