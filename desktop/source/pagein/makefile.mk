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

PRJ=..$/..

PRJNAME=desktop
TARGET=pagein
TARGETTYPE=CUI
LIBTARGET=NO

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.INCLUDE .IGNORE : icuversion.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/pagein.obj \
    $(OBJ)$/pagein-main.obj \
    $(OBJ)$/file_image_unx.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1CODETYPE=C

# depends on libc only.
STDLIB=

# --- Targets ------------------------------------------------------

ALL: \
    $(MISC)$/$(TARGET)-calc    \
    $(MISC)$/$(TARGET)-draw    \
    $(MISC)$/$(TARGET)-impress \
    $(MISC)$/$(TARGET)-writer  \
    $(MISC)$/$(TARGET)-common  \
    ALLTAR

.INCLUDE :  target.mk

ICUDLLPOST=$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR)
UDKDLLPOST=$(DLLPOST).$(UDK_MAJOR)
UNODLLPOST=.uno$(DLLPOST)
DFTDLLPOST=$(DLLPOSTFIX)$(DLLPOST) # Default 

URELIBPATH=..$/ure-link$/lib
UREMISCPATH=..$/ure-link$/share$/misc

$(MISC)$/$(TARGET)-calc : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sc$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)scui$(DFTDLLPOST) >>  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
    @-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-draw : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)sdui$(DFTDLLPOST) >>  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
    @-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-impress : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)sdui$(DFTDLLPOST) >>  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
    @-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-writer : makefile.mk
    @echo Making: $@
    @-echo $(DLLPRE)sw$(DFTDLLPOST)  >  $@
    @-echo $(DLLPRE)swui$(DFTDLLPOST) >>  $@
    @-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
    @-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

# sorted in approx. reverse load order (ld.so.1)
$(MISC)$/$(TARGET)-common : makefile.mk
    @echo Making: $@
    @-echo i18npool$(UNODLLPOST)         >  $@
.IF "$(SYSTEM_ICU)" != "YES"
    @-echo $(DLLPRE)icui18n$(ICUDLLPOST) >> $@
    @-echo $(DLLPRE)icule$(ICUDLLPOST)   >> $@
    @-echo $(DLLPRE)icuuc$(ICUDLLPOST)   >> $@
#   @-echo $(DLLPRE)icudata$(ICUDLLPOST) >> $@ - a huge dll, almost none of it used
.ENDIF # SYSTEM_ICU
#
    @-echo $(DLLPRE)lng$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)xo$(DFTDLLPOST)    >> $@
#

    @-echo $(DLLPRE)fwe$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)fwk$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)fwi$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)fwl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)package2$(DLLPOST) >> $@
    @-echo $(DLLPRE)ucpfile1$(DLLPOST) >> $@
    @-echo $(DLLPRE)ucb1$(DLLPOST)     >> $@
    @-echo configmgr$(UNODLLPOST)      >> $@
#
    @-echo $(DLLPRE)vclplug_gen$(DFTDLLPOST) >> $@
.IF "$(ENABLE_GTK)" != ""
    @-echo $(DLLPRE)vclplug_gtk$(DFTDLLPOST) >> $@
.ENDIF # ENABLE_GTK
.IF "$(ENABLE_KDE)" != ""
    @-echo $(DLLPRE)vclplug_kde$(DFTDLLPOST) >> $@
.ENDIF # ENABLE_KDE
#
    @-echo $(DLLPRE)basegfx$(DFTDLLPOST) >> $@
    @-echo $(DLLPRE)sot$(DFTDLLPOST)     >> $@
    @-echo $(DLLPRE)xcr$(DFTDLLPOST)     >> $@
    @-echo $(DLLPRE)sb$(DFTDLLPOST)      >> $@
#
# uno runtime environment
#
    @-echo $(URELIBPATH)$/stocservices$(UNODLLPOST)         >> $@
    @-echo $(URELIBPATH)$/bootstrap$(UNODLLPOST)            >> $@
    @-echo $(URELIBPATH)$/$(DLLPRE)reg$(UDKDLLPOST)         >> $@
    @-echo $(URELIBPATH)$/$(DLLPRE)store$(UDKDLLPOST)       >> $@
    @-echo $(URELIBPATH)$/reflection$(UNODLLPOST)           >> $@
    @-echo $(URELIBPATH)$/$(DLLPRE)uno_cppuhelper$(COMID)$(UDKDLLPOST) >> $@
    @-echo $(URELIBPATH)$/$(DLLPRE)uno_cppu$(UDKDLLPOST)               >> $@
    @-echo $(URELIBPATH)$/$(DLLPRE)uno_sal$(UDKDLLPOST)                >> $@
#
    @-echo $(DLLPRE)ucbhelper$(UCBHELPER_MAJOR)$(COMID)$(DLLPOST) >> $@
    @-echo $(DLLPRE)comphelp$(COMPHLP_MAJOR)$(COMID)$(DLLPOST)    >> $@
    @-echo $(DLLPRE)tl$(DFTDLLPOST)    >> $@
    @-echo $(DLLPRE)utl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)svl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)vcl$(DFTDLLPOST)   >> $@
    @-echo $(DLLPRE)tk$(DFTDLLPOST)    >> $@
    @-echo $(UREMISCPATH)$/types.rdb   >> $@
    @-echo services.rdb                >> $@
    @-echo oovbaapi.rdb                          >> $@
    @-echo deployment$(DLLPOSTFIX)$(UNODLLPOST)  >> $@
    @-echo $(DLLPRE)deploymentmisc$(DFTDLLPOST)  >> $@
    @-echo $(DLLPRE)ucb1$(DLLPOST)               >> $@
    @-echo $(DLLPRE)xstor$(DLLPOST)              >> $@
    @-echo $(DLLPRE)package2$(DLLPOST)           >> $@
    @-echo $(DLLPRE)filterconfig1$(DLLPOST)      >> $@
    @-echo $(DLLPRE)uui$(DFTDLLPOST)             >> $@
    @-echo $(DLLPRE)lng$(DFTDLLPOST)             >> $@
    @-echo $(DLLPRE)svt$(DFTDLLPOST)             >> $@
    @-echo $(DLLPRE)spl$(DFTDLLPOST)             >> $@
    @-echo $(DLLPRE)basegfx$(DFTDLLPOST)         >> $@
    @-echo $(DLLPRE)avmedia$(DFTDLLPOST)         >> $@
    @-echo $(DLLPRE)helplinker$(DFTDLLPOST)      >> $@
    @-echo $(DLLPRE)vclplug_gen$(DFTDLLPOST)     >> $@
    @-echo $(DLLPRE)icule$(ICUDLLPOST)           >> $@
    @-echo sax$(UNODLLPOST)                      >> $@
    @-echo gconfbe1$(UNODLLPOST)                 >> $@
    @-echo fsstorage$(UNODLLPOST)                >> $@
    @-echo desktopbe1$(UNODLLPOST)               >> $@
    @-echo localebe1$(UNODLLPOST)                >> $@
    @-echo ucpexpand1$(UNODLLPOST)               >> $@
# stoc bits
    @-echo $(DLLPRE)sfx$(DFTDLLPOST)             >> $@
    @-echo $(DLLPRE)sofficeapp$(DLLPOST)         >> $@
