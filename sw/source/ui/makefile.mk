#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sw
TARGET=ui

PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=.$/ui_1st$/ui_pch

.IF "$(CALLTARGETS)"=="ui"
RC_SUBDIRS=
.ENDIF

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
.IF "$(RC_SUBDIRS)" == ""
SWSUBDIRS= \
        ui_1st \
        app      \
        cctrl    \
        chrdlg   \
        config   \
        dbui     \
        dialog   \
        dochdl   \
        docvw    \
        envelp   \
        fldui    \
        fmtui    \
        frmdlg   \
        globdoc  \
        index    \
        lingu    \
        misc     \
        ribbar   \
        shells   \
        table    \
        uno	\
        uiview   \
        utlui    \
        web	  \
        wizard	  \
        wrtsh
.ENDIF

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
        $(SLB)$/fmtui.lib    \
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
        $(SLB)$/wizard.lib   \
        $(SLB)$/uiview.lib   \
        $(SLB)$/wrtsh.lib

# -----------------------------------------------------------

.IF "$(RC_SUBDIRS)" == ""
.IF "$(depend)" == ""
ui:						\
    ui_1st				\
    $(SWSUBDIRS) \
    ALLTAR
.ELSE
ui:
    @+echo Doing nothing in source\ui
.ENDIF
.ENDIF

################################################################

LIB1TARGET=$(SLB)$/ui1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/ui2.lib
LIB2FILES= \
        $(SUBLIBS2)

.INCLUDE :  target.mk

################################################################

.IF "$(dbutil)" != ""
dbutilx="dbutil=true"
.ENDIF
.IF "$(debug)" != ""
.IF "$(debug)"!="D_FORCE_OPT"
dbutilx="debug=$(debug)"
.ENDIF
.ENDIF

.IF "$(compact)" != ""
dcompact="compact=true"
.ELSE
dcompact=
.ENDIF

.IF "$(CALLTARGETS)"!="ui"
.IF "$(DOPLD)$(dopld)" == ""

.IF "$(RC_SUBDIRS)" == ""
#rule lib / subdir
$(LB)$/%.lib : %
    @echo @

ui_1st .SETDIR=ui_1st:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS)  $(dbutilx) $(CALLMACROS)

#	app $(LB)$/app.lib .SETDIR=app:
app .SETDIR=app:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

cctrl  .SETDIR=cctrl:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

chrdlg  .SETDIR=chrdlg:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

config  .SETDIR=config:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

dbui  .SETDIR=dbui:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

dialog  .SETDIR=dialog:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

dochdl  .SETDIR=dochdl:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

docvw  .SETDIR=docvw:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

envelp  .SETDIR=envelp:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

fldui  .SETDIR=fldui:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

fmtui  .SETDIR=fmtui:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

frmdlg  .SETDIR=frmdlg:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

globdoc  .SETDIR=globdoc:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

index  .SETDIR=index:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

lingu  .SETDIR=lingu:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

misc  .SETDIR=misc:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

ribbar  .SETDIR=ribbar:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

shells  .SETDIR=shells:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

table  .SETDIR=table:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

uiview  .SETDIR=uiview:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

utlui  .SETDIR=utlui:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

uno  .SETDIR=uno:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

wizard  .SETDIR=wizard:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

web  .SETDIR=web:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)

wrtsh  .SETDIR=wrtsh:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(dcompact) $(CALLMACROS)
.ENDIF
.ELSE
ui_1st:
    @echo nix
.ENDIF
.ENDIF

kill:
    @+$(RM) $(SLB)$/ui.lst
    @+$(RM) $(SLB)$/ui.lib


