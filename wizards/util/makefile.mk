#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:06:31 $
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

PRJ=..

PRJNAME=wizards
TARGET=NOTARGET
GEN_HID=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk



SBLLINK=sbllink

.IF "$(depend)" == ""
all:    \
    $(SRS)$/hidother.hid    \
    mksbldir \
    link \
    printlog \
    ALLTAR

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)$(CPU)"=="WNTI"
    @+copy hidother.src ..$/$(INPATH)$/srs$/hidother.hid
.ELSE
    @+echo nix
.ENDIF

.IF "$(GUI)"!="WNT"


klausbl:
    +$(COPY) $(UPDATE) $(PRJ)$/wntmsci.pro$/sbl$/*.* $(PRJ)$/$(INPATH)$/sbl

.ENDIF                  # "$(GUI)"!="WNT"

.ENDIF # no depend
# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

link: \
    $(MISC)$/soffice.lnk \
    mksbldir
    $(SBLLINK) /Link:$(MISC)$/soffice.lnk
    @echo Log File:
    @+-$(TYPE) $(MISC)$/*.log

dump: \
    $(MISC)$/soffice.lnk
    $(SBLLINK) /Dump:$(MISC)$/soffice.lnk

dumpout: \
    $(MISC)$/soffice.lnk
    $(SBLLINK) /Dump:$(MISC)$/soffice.lnk
    @echo Log File:
    @+-$(TYPE) $(MISC)$/*.log

kill:
    del $(OUT)$/sbl\

killall: kill


dummytarget:
    @echo done > $(MISC)$/dummy.don

mksbldir :
    +-$(MKDIRHIER) $(PRJ)$/$(INPATH)$/sbl >& $(NULLDEV)

printlog :
    @echo Log File:
    @+-$(TYPE) $(MISC)$/*.log

$(MISC)$/soffice.lnk : soffice.lnk
.IF "$(GUI)"=="UNX"
    +-tr -d "\015" < $< > $@
.ELSE
    @+$(COPY) soffice.lnk $@
.ENDIF

$(MISC)$/$(TARGET).hid: $(BIN)$/iwz$(UPD)49.res

