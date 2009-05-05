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

PRJNAME=migrationanalysis
TARGET=wizard

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

PAW_APPSDEST:=$(BIN)$/ProAnalysisWizard
ALTERNATE_SRC:=..$/exe

PAW_DATDEST:=$(PAW_APPSDEST)$/Resources

PROJECTDEST:=$(BIN)$/ProAnalysisWizard
RCFILES:=$(RES)$/$(TARGET).rc
RESFILE:=$(RES)$/$(TARGET).res
ULFFILES:=$(TARGET).ulf
.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""
MANIFEST:=$(RES)$/DocAnalysisWizard.exe.manifest

LAUNCHER_APP:= $(BIN)$/LaunchDrivers.exe

BASIC_APPS:= \
    $(LAUNCHER_APP) \
    $(PAW_APPSDEST)$/ProAnalysisWizard.exe

BASIC_VBP:= \
    $(BIN)$/LaunchDrivers.vbp \
    $(PAW_APPSDEST)$/ProAnalysisWizard.vbp

PAW_RES_DLLS:=$(PAW_DATDEST)$/Resources.dll
PAW_LAUNCHER:=$(PAW_DATDEST)$/LaunchDrivers.exe

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(BASIC_APPS) $(PAW_RES_DLLS) $(PAW_LAUNCHER)

$(MANIFEST) : $$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.IF "$(VB6_LOCATION)" != ""

$(BASIC_VBP) : $$(@:b).vbp
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(@:f) $@

$(BASIC_APPS) : $(BASIC_VBP)
    -$(MKDIRHIER) $(@:d)
    cd $(@:d) && "$(VB6_LOCATION)$/vb6.exe" /m $(@:b).vbp

$(PAW_LAUNCHER) : $(LAUNCHER_APP)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(LAUNCHER_APP) $@

.ELSE          # "$(VB6_LOCATION)" != ""

$(BASIC_APPS) : $(ALTERNATE_SRC)$/$$(@:f)
    @echo "------------------------------------"
    @echo "No VB6 found: using prebuild wizards"
    @echo "------------------------------------"
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(ALTERNATE_SRC)$/$(@:f) $@

$(PAW_LAUNCHER) : $(ALTERNATE_SRC)$/$$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(ALTERNATE_SRC)$/$(@:f) $@

.ENDIF          # "$(VB6_LOCATION)" != ""

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(MANIFEST) $(ULFDIR)$/$(TARGET).ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(WRAPCMD) lngconvex.exe -ulf $(ULFDIR)$/$(TARGET).ulf -rc $(RES)$/$(TARGET).rc -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

$(PAW_RES_DLLS) : $(RCFILES)
    -$(MKDIRHIER) $(@:d)
    link /NOENTRY /DLL /MACHINE:X86 /OUT:$@ $(RESFILE)

