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

PRJ=..
PRJNAME=forms
TARGET=frm
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk
.INCLUDE: $(PRJ)$/makefile.pmk

# --- Library -----------------------------------
# --- frm ---------------------------------------
LIB1TARGET=$(SLB)$/forms.lib
LIB1FILES=\
        $(SLB)$/common.lib \
        $(SLB)$/resource.lib \
        $(SLB)$/component.lib \
        $(SLB)$/helper.lib \
        $(SLB)$/solarcomponent.lib  \
        $(SLB)$/solarcontrol.lib \
        $(SLB)$/richtext.lib \
        $(SLB)$/runtime.lib \
        $(SLB)$/xforms.lib \
        $(SLB)$/xformssubmit.lib \
        $(SLB)$/xformsxpath.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(EDITENGLIB) \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(VCLLIB) \
        $(SVTOOLLIB) \
        $(SVLLIB)	\
        $(TKLIB) \
        $(SFX2LIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(DBTOOLSLIB) \
        $(TKLIB) \
        $(SVXCORELIB) \
        $(UCBHELPERLIB) \
        $(LIBXML2LIB) \
        $(ICUUCLIB) \
        $(ICUINLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEPN=$(LIB1TARGET)	\
        makefile.mk


SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# === .res file ==========================================================

RES1FILELIST=\
    $(SRS)$/resource.srs \

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

.IF "$(GUI)"=="OS2"
SHL1STDLIBS += pthread.lib libz.lib
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo __CT				    >$@
    @echo createRegistryInfo    >>$@
    @echo queryInterface        >>$@
    @echo queryAggregation      >>$@
    @echo NavigationToolBar     >>$@
    @echo ONavigationBar        >>$@


ALLTAR : $(MISC)/frm.component

$(MISC)/frm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        frm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt frm.component
