#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    $(LNGCONVEX) -ulf $(ULFDIR)$/$(TARGET).ulf -rc $(RES)$/$(TARGET).rc -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

$(PAW_RES_DLLS) : $(RCFILES)
    -$(MKDIRHIER) $(@:d)
    link /NOENTRY /DLL /MACHINE:X86 /OUT:$@ $(RESFILE)

