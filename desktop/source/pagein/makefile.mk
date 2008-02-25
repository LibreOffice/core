#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:49:48 $
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
DFTDLLPOST=$(DLLPOSTFIX)$(DLLPOST) # Default 

$(MISC)$/$(TARGET)-calc : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sc$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-draw : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-impress : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-writer : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sw$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@

# sorted in reverse load order (ld.so.1)
$(MISC)$/$(TARGET)-common : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)icui18n$(ICUDLLPOST)  >  $@
    @-echo i18npool$(UNODLLPOST)       >> $@
#
    @-echo $(DLLPRE)xcr$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)xo$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)go$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)sb$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)sfx$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)so$(DFTDLLPOST)    >> $@
#
    @-echo $(DLLPRE)fwe$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)fwk$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)ucpfile1$(DLLPOST) >> $@
    @-echo $(DLLPRE)fwi$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)fwl$(DFTDLLPOST)   >> $@
    @-echo configmgr2$(UNODLLPOST)     >> $@
#
    @-echo $(DLLPRE)icuuc$(ICUDLLPOST) >> $@
    @-echo $(DLLPRE)sot$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)psp$(DFTDLLPOST)   >> $@
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
    @-echo $(DLLPRE)tl$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)utl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)svl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)vcl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)tk$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)svt$(DFTDLLPOST)   >> $@
    @-echo soffice.bin                 >> $@
