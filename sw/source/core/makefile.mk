#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

TARGET=core

PROJECTPCH=core_pch
PDBTARGET=core_pch
PROJECTPCHSOURCE=.\core_1st\core_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(CALLTARGETS)"=="core"
RC_SUBDIRS=
.ENDIF

.IF "$(GUI)" != "WNT"
.IF "$(GUI)" != "MAC"
.IF "$(COM)" != "BLC"
.IF "$(COM)" != "WTC"
LIBFLAGS=/NOI /NOE /PAGE:256
.ENDIF
.ENDIF
.ENDIF
.ENDIF

.IF "$(COM)"=="WTC"
LIBFLAGS=$(LIBFLAGS) /p=256
.ENDIF

# --- Files --------------------------------------------------------

# fuer VC++/NT andere Label als die Verzeichnisnamen
.IF "$(RC_SUBDIRS)" == ""
SWSUBDIRS= \
    core_1st \
    attr \
    bastyp \
    crsr \
    doc \
    docnode \
    draw \
    edit \
    fields \
    frmedt \
    graphic \
    layout \
    ole \
    para \
    sw3io \
    swg \
    text \
    tox \
    txtnode \
       unocore \
    undo \
    view

.IF "$(PRODUCT)" == ""
SWSUBDIRS+= \
    except
.ENDIF
.IF "$(COM)$(GUI)"=="MSCWIN"
SWSUBDIRS+= \
    rtlfix
.ENDIF
.ENDIF

SUBLIBS1= \
        $(SLB)$/graphic.lib \
        $(SLB)$/para.lib \
        $(SLB)$/attr.lib \
        $(SLB)$/edit.lib \
        $(SLB)$/crsr.lib \
        $(SLB)$/view.lib \
        $(SLB)$/frmedt.lib \
        $(SLB)$/ole.lib \
        $(SLB)$/fields.lib \
        $(SLB)$/tox.lib \
        $(SLB)$/undo.lib \
        $(SLB)$/bastyp.lib


SUBLIBS2= \
        $(SLB)$/draw.lib \
        $(SLB)$/sw3io.lib \
        $(SLB)$/swg.lib \
        $(SLB)$/layout.lib \
        $(SLB)$/text.lib \
        $(SLB)$/txtnode.lib \
        $(SLB)$/doc.lib \
        $(SLB)$/docnode.lib \
        $(SLB)$/unocore.lib

.IF "$(PRODUCT)" == ""
SUBLIBS2+= \
        $(SLB)$/except.lib
.ENDIF

#-------------------------------------------------------------------------

.IF "$(RC_SUBDIRS)" == ""
.IF "$(depend)" == ""
core:						\
    core_1st				\
    $(SWSUBDIRS)				\
    ALLTAR
.ELSE
core:
    @+echo Doing nothing in source\core
.ENDIF
.ENDIF

################################################################

LIB1TARGET=$(SLB)$/core1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/core2.lib
LIB2FILES= \
        $(SUBLIBS2)

.INCLUDE :  target.mk

################################################################

#-------------------------------------------------------------------------

.IF "$(dbutil)" != ""
dbutilx="dbutil=true"
.ENDIF
.IF "$(debug)" != ""
.IF "$(debug)" != "D_FORCE_OPT"
dbutilx="debug=true"
.ENDIF
.ENDIF

#-------------------------------------------------------------------------

.IF "$(CALLTARGETS)"!="core"
.IF "$(DOPLD)$(dopld)" == ""

.IF "$(RC_SUBDIRS)" == ""
#rule lib / subdir
$(LB)$/%.lib : %
    @echo @

core_1st .SETDIR=core_1st:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

attr .SETDIR=attr:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

bastyp .SETDIR=bastyp:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

crsr .SETDIR=crsr:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

doc .SETDIR=doc:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

docnode .SETDIR=docnode:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

draw .SETDIR=draw:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

edit .SETDIR=edit:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

except .SETDIR=except:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

fields .SETDIR=fields:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

frmedt .SETDIR=frmedt:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

graphic .SETDIR=graphic:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

layout .SETDIR=layout:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

.IF "$(SOLAR_JAVA)" != ""
javascrp .SETDIR=javascrp:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)
.ENDIF

ole .SETDIR=ole:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

para .SETDIR=para:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

rtlfix .SETDIR=rtlfix:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

sw3io .SETDIR=sw3io:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

swg .SETDIR=swg:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

text .SETDIR=text:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

tox .SETDIR=tox:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

txtnode .SETDIR=txtnode:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

undo .SETDIR=undo:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

unocore .SETDIR=unocore:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

view .SETDIR=view:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

.ENDIF
.ELSE
core_1st:
    @echo nix
.ENDIF
.ENDIF

kill:
    +-$(RM) $(SLB)$/core1.lib
    +-$(RM) $(SLB)$/core2.lib

