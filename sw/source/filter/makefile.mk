#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
TARGET=filter

PROJECTPCH=filt_pch
PDBTARGET=filt_pch
PROJECTPCHSOURCE=.\filt_1st\filt_pch

.IF "$(CALLTARGETS)"=="filter"
RC_SUBDIRS=
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

# fuer VC++/NT andere Label als Verzeichnisnamen
.IF "$(RC_SUBDIRS)" == ""
SWSUBDIRS= \
    filt_1st \
    ascii \
    basflt \
    excel \
    html \
    lotus \
    rtf \
    w4w \
    writer \
    ww1 \
    ww8 \
    xml

.IF "$(product)" == ""
.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT" || "$(GUI)" == "UNX"
SWSUBDIRS+= \
    debug
.ENDIF
.ENDIF

.IF "$(compact)" == ""
SWSUBDIRS+= \
    sw6
.ENDIF
.ENDIF

SUBLIBS= \
    $(SLB)$/ascii.lib \
    $(SLB)$/basflt.lib \
    $(SLB)$/excel.lib \
    $(SLB)$/html.lib \
    $(SLB)$/lotus.lib \
    $(SLB)$/rtf.lib \
    $(SLB)$/w4w.lib \
    $(SLB)$/writer.lib \
    $(SLB)$/ww1.lib \
    $(SLB)$/ww8.lib \
    $(SLB)$/xml.lib

.IF "$(product)" == ""
.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT" || "$(GUI)" == "UNX"
SUBLIBS+= \
    $(SLB)$/debug.lib
.ENDIF
.ENDIF
.IF "$(compact)" == ""
SUBLIBS+= \
     $(SLB)$/sw6.lib
.ENDIF

# -----------------------------------------------------------

.IF "$(RC_SUBDIRS)" == ""
.IF "$(depend)" == ""
filter: 					\
    filt_1st				\
    $(SWSUBDIRS)				\
    ALLTAR
.ELSE
filter:
    @+echo Doing nothing in source\filter
.ENDIF
.ENDIF

################################################################

LIB1TARGET=$(SLB)$/filter.lib
LIB1FILES= \
        $(SUBLIBS)

.INCLUDE :	target.mk

################################################################

.IF "$(dbutil)" != ""
dbutilx += "dbutil=true"
.ENDIF
.IF "$(debug)" != ""
.IF "$(debug)" != "D_FORCE_OPT"
dbutilx += "debug=true"
.ENDIF
.ENDIF


.IF "$(CALLTARGETS)"!="filter"
.IF "$(DOPLD)$(dopld)" == ""

.IF "$(RC_SUBDIRS)" == ""
#rule lib / subdir
$(SLB)$/%.lib : %
    @echo @

filt_1st .SETDIR=filt_1st:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

ascii .SETDIR=ascii:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

basflt .SETDIR=basflt:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

debug .SETDIR=debug:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

excel .SETDIR=excel:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

html .SETDIR=html:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

lotus .SETDIR=lotus:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

rtf .SETDIR=rtf:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

sw6 .SETDIR=sw6:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

w4w .SETDIR=w4w:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

writer .SETDIR=writer:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

ww1 .SETDIR=ww1:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

ww8 .SETDIR=ww8:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

xml .SETDIR=xml:
        @echo $@
        @$(MAKECMD) -d $(MFLAGS) $(dbutilx) $(CALLMACROS)

.ENDIF
.ELSE
filt_1st:
    @echo nix
.ENDIF
.ENDIF

kill:
    del $(SLB)$/filter.lib

