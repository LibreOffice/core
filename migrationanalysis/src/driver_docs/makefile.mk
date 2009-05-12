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
# $Revision: 1.8.34.3 $
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

