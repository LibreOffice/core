#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2006-10-05 10:47:06 $
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

ICUDLLPOST=$(DLLPOST).26
UDKDLLPOST=$(DLLPOST).$(UDK_MAJOR)
UNODLLPOST=.uno$(DLLPOST)
UPDDLLPOST=$(UPD)$(DLLPOSTFIX)$(DLLPOST)

$(MISC)$/$(TARGET)-calc : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sc$(UPDDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(UPDDLLPOST) >> $@

$(MISC)$/$(TARGET)-draw : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(UPDDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(UPDDLLPOST) >> $@

$(MISC)$/$(TARGET)-impress : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(UPDDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(UPDDLLPOST) >> $@

$(MISC)$/$(TARGET)-writer : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sw$(UPDDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(UPDDLLPOST) >> $@

# sorted in reverse load order (ld.so.1)
$(MISC)$/$(TARGET)-common : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)icui18n$(ICUDLLPOST)  >  $@
    @-echo i18npool$(UNODLLPOST)       >> $@
#
    @-echo $(DLLPRE)xcr$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)xo$(UPDDLLPOST)    >> $@
    @-echo $(DLLPRE)go$(UPDDLLPOST)    >> $@
    @-echo $(DLLPRE)sb$(UPDDLLPOST)    >> $@
    @-echo $(DLLPRE)sfx$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)so$(UPDDLLPOST)    >> $@
#
    @-echo $(DLLPRE)fwe$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)fwk$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)ucpfile1$(DLLPOST) >> $@
    @-echo $(DLLPRE)fwi$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)fwl$(UPDDLLPOST)   >> $@
    @-echo configmgr2$(UNODLLPOST)     >> $@
#
    @-echo $(DLLPRE)icuuc$(ICUDLLPOST) >> $@
    @-echo $(DLLPRE)sot$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)psp$(UPDDLLPOST)   >> $@
.IF "$(COMNAME)" == "gcc2" || "$(COMNAME)" == "gcc3"
    @-echo $(DLLPRE)stlport_gcc$(DLLPOST)    >> $@
.ENDIF # gcc
.IF "$(COMNAME)" == "sunpro5"
    @-echo $(DLLPRE)stlport_sunpro$(DLLPOST) >> $@
.ENDIF # sunpro5
    @-echo $(DLLPRE)uno_sal$(UDKDLLPOST)   >>  $@
    @-echo $(DLLPRE)uno_cppu$(UDKDLLPOST)  >> $@
    @-echo $(DLLPRE)uno_cppuhelper$(COMID)$(UDKDLLPOST)           >> $@
    @-echo $(DLLPRE)ucbhelper$(UCBHELPER_MAJOR)$(COMID)$(DLLPOST) >> $@
    @-echo $(DLLPRE)comphelp$(COMPHLP_MAJOR)$(COMID)$(DLLPOST)    >> $@
    @-echo $(DLLPRE)tl$(UPDDLLPOST)    >> $@
    @-echo $(DLLPRE)utl$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)svl$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)vcl$(UPDDLLPOST)   >> $@
    @-echo $(DLLPRE)tk$(UPDDLLPOST)    >> $@
    @-echo $(DLLPRE)svt$(UPDDLLPOST)   >> $@
    @-echo soffice.bin                 >> $@
