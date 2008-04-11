#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.11 $
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
