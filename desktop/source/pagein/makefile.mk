#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 15:48:34 $
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

PRJNAME=desktop
TARGET=pagein
TARGETTYPE=CUI
LIBTARGET=NO

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/pagein.obj \
    $(OBJ)$/file_image_unx.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)

# depends on libc only.
STDLIB=
.IF "$(COMNAME)" == "sunpro5"
STDLIB+=-library=no%Crun
.ENDIF # sunpro5

# --- Targets ------------------------------------------------------

ALL: \
    $(MISC)$/$(TARGET)-calc    \
    $(MISC)$/$(TARGET)-draw    \
    $(MISC)$/$(TARGET)-impress \
    $(MISC)$/$(TARGET)-writer  \
    $(MISC)$/$(TARGET)-common  \
    ALLTAR

.INCLUDE :  target.mk

ICUDLLPOST=$(DLLPOST).22
UDKDLLPOST=$(DLLPOST).$(UDK_MAJOR)
UPDDLLPOST=$(UPD)$(DLLPOSTFIX)$(DLLPOST)

$(MISC)$/$(TARGET)-calc : makefile.mk
    @+echo Making: $@
    @+echo $(DLLPRE)sc$(UPDDLLPOST)  >  $@

$(MISC)$/$(TARGET)-draw : makefile.mk
    @+echo Making: $@
    @+echo $(DLLPRE)sd$(UPDDLLPOST)  >  $@

$(MISC)$/$(TARGET)-impress : makefile.mk
    @+echo Making: $@
    @+echo $(DLLPRE)sd$(UPDDLLPOST)  >  $@

$(MISC)$/$(TARGET)-writer : makefile.mk
    @+echo Making: $@
    @+echo $(DLLPRE)sw$(UPDDLLPOST)  >  $@

# sorted in reverse load order (ld.so.1)
$(MISC)$/$(TARGET)-common : makefile.mk
    @+echo Making: $@
    @-+echo $(DLLPRE)icui18n$(ICUDLLPOST)  >  $@
    @-+echo $(DLLPRE)i18npool$(UPDDLLPOST) >> $@
#
    @-+echo $(DLLPRE)xcr$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)xo$(UPDDLLPOST)    >> $@
    @-+echo $(DLLPRE)go$(UPDDLLPOST)    >> $@
    @-+echo $(DLLPRE)sb$(UPDDLLPOST)    >> $@
    @-+echo $(DLLPRE)sfx$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)svx$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)ofa$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)so$(UPDDLLPOST)    >> $@
#
    @-+echo $(DLLPRE)fwe$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)fwk$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)ucpfile1$(DLLPOST) >> $@
    @-+echo $(DLLPRE)fwi$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)fwl$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)cfgmgr2$(DLLPOST)  >> $@
#
    @-+echo $(DLLPRE)icuuc$(ICUDLLPOST) >> $@
    @-+echo $(DLLPRE)sot$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)psp$(UPDDLLPOST)   >> $@
.IF "$(COMNAME)" == "gcc2" || "$(COMNAME)" == "gcc3"
    @-+echo $(DLLPRE)stlport_gcc$(DLLPOST)    >> $@
.ENDIF # gcc
.IF "$(COMNAME)" == "sunpro5"
    @-+echo $(DLLPRE)stlport_sunpro$(DLLPOST) >> $@
.ENDIF # sunpro5
    @-+echo $(DLLPRE)sal$(UDKDLLPOST)   >>  $@
    @-+echo $(DLLPRE)cppu$(UDKDLLPOST)  >> $@
    @-+echo $(DLLPRE)cppuhelper$(COMID)$(UDKDLLPOST)               >> $@
    @-+echo $(DLLPRE)ucbhelper$(UCBHELPER_MAJOR)$(COMID)$(DLLPOST) >> $@
    @-+echo $(DLLPRE)comphelp$(COMPHLP_MAJOR)$(COMID)$(DLLPOST)    >> $@
    @-+echo $(DLLPRE)tl$(UPDDLLPOST)    >> $@
    @-+echo $(DLLPRE)utl$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)svl$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)vcl$(UPDDLLPOST)   >> $@
    @-+echo $(DLLPRE)tk$(UPDDLLPOST)    >> $@
    @-+echo $(DLLPRE)svt$(UPDDLLPOST)   >> $@
    @-+echo soffice.bin                 >> $@
