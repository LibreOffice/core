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
TARGET=driverdocs

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

PAW_DOCDEST:=$(BIN)$/ProAnalysisWizard$/Resources
PAW_DATDEST:=$(BIN)$/ProAnalysisWizard$/Resources$/lang

PAW_DRIVER_DOCS:= \
    $(PAW_DOCDEST)$/_OOoDocAnalysisExcelDriver.xls \
    $(PAW_DOCDEST)$/_OOoDocAnalysisWordDriver.doc \
    $(PAW_DOCDEST)$/_OOoDocAnalysisPPTDriver.ppt

PAW_DRIVER_DOCS_SRC:= \
    .$/PAW$/_OOoDocAnalysisExcelDriver.xls \
    .$/PAW$/_OOoDocAnalysisWordDriver.doc \
    .$/PAW$/_OOoDocAnalysisPPTDriver.ppt

PAW_DAT_FILES= \
    $(foreach,i,$(alllangiso) $(PAW_DATDEST)$/$i.dat)

DAT_DATA_FILE=allstrings.ulf
DAT_DON_FILE_PAW=$(MISC)$/$(DAT_DATA_FILE).paw

ULFFILES=$(DAT_DATA_FILE)
.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

COMMON_SRC:= \
    .$/sources$/AnalysisDriver.bas \
    .$/sources$/CollectedFiles.cls \
    .$/sources$/CommonMigrationAnalyser.bas \
    .$/sources$/CommonPreparation.bas \
    .$/sources$/DocumentAnalysis.cls \
    .$/sources$/FileTypeAssociation.cls \
    .$/sources$/IssueInfo.cls \
    .$/sources$/LocalizeResults.bas \
    .$/sources$/PrepareInfo.cls \
    .$/sources$/StringDataManager.cls \
    .$/sources$/Stripped_OOoDocAnalysisExcelDriver.xls \
    .$/sources$/Stripped_OOoDocAnalysisPPTDriver.ppt \
    .$/sources$/Stripped_OOoDocAnalysisWordDriver.doc \
    .$/sources$/common_res.bas \
    .$/sources$/results_res.bas

EXCEL_SRC:= \
    .$/sources$/excel$/ApplicationSpecific.bas \
    .$/sources$/excel$/MigrationAnalyser.cls \
    .$/sources$/excel$/Preparation.bas \
    .$/sources$/excel$/excel_res.bas

PP_SRC:= \
    .$/sources$/powerpoint$/ApplicationSpecific.bas \
    .$/sources$/powerpoint$/MigrationAnalyser.cls \
    .$/sources$/powerpoint$/Preparation.bas \
    .$/sources$/powerpoint$/powerpoint_res.bas

WORD_SRC:= \
    .$/sources$/word$/ApplicationSpecific.bas \
    .$/sources$/word$/MigrationAnalyser.cls \
    .$/sources$/word$/Preparation.bas \
    .$/sources$/word$/word_res.bas

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(PAW_DRIVER_DOCS) $(DAT_DON_FILE_PAW)

$(PAW_DRIVER_DOCS) : .$/PAW$/$$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) .$/PAW$/$(@:f) $@

$(PAW_DAT_FILES) : $(ULFDIR)$/$(DAT_DATA_FILE)
    -$(MKDIRHIER) $(@:d)
    $(TOUCH) $@

$(DAT_DON_FILE_PAW) :  $(PAW_DAT_FILES)
    @echo --------------------------------
    @echo building $@
    -$(MKDIRHIER) $(@:d)
    @echo making  $(PAW_DAT_FILES)
    $(PERL) ulf2dat.pl -i $(ULFDIR)$/$(DAT_DATA_FILE) $(PAW_DAT_FILES) && $(TOUCH) $@

.IF "$(VB6_LOCATION)" != ""
$(PAW_DRIVER_DOCS_SRC) : $(COMMON_SRC) $(EXCEL_SRC) $(PP_SRC) $(WORD_SRC)
    @echo --------------------------------
    @echo create driver docs
    cscript CreateDriverDocs.wsf
.ENDIF

