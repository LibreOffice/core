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

PRJ=..$/..$/..$/..
PRJNAME=sdext
TARGET=testdocs

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE : target.mk

TESTFILES=\
    graphicformats.pdf \
    pictxt.pdf \
    txtpic.pdf \
    txtpictxt.pdf \
    verticaltext.pdf

# --- Fake uno bootstrap ------------------------
.IF "$(ENABLE_PDFIMPORT)" == "TRUE"

$(BIN)$/pdfi_unittest_test.ini : makefile.mk
    rm -f $@
    @echo UNO_SERVICES= > $@
    @echo UNO_TYPES=$(UNOUCRRDB:s/\/\\/) >> $@

# --- Enable test execution in normal build, diff all test docs ---

ALLTAR : $(BIN)$/pdfi_unittest_test.ini \
         $(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_writer_succeeded/:f) $(MISC)$/$i) \
         $(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_impress_succeeded/:f) $(MISC)$/$i) \
         $(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_draw_succeeded/:f) $(MISC)$/$i)

$(MISC)$/%_pdfi_unittest_writer_succeeded : $$(@:s/_succeeded/.xml/:f)
    rm -f $(MISC)$/$(@:s/_succeeded/.xml/:f)
    $(BIN)$/pdf2xml -writer $(@:s/_pdfi_unittest_writer_succeeded/.pdf/:f) $(MISC)$/$(@:s/_succeeded/.xml/:f) $(BIN)$/pdfi_unittest_test.ini
    diff $(MISC)$/$(@:s/_succeeded/.xml/:f) $(@:s/_succeeded/.xml/:f)
    $(TOUCH) $@
$(MISC)$/%_pdfi_unittest_impress_succeeded : $$(@:s/_succeeded/.xml/:f)
    rm -f $(MISC)$/$(@:s/_succeeded/.xml/:f)
    $(BIN)$/pdf2xml -impress $(@:s/_pdfi_unittest_impress_succeeded/.pdf/:f) $(MISC)$/$(@:s/_succeeded/.xml/:f) $(BIN)$/pdfi_unittest_test.ini
    diff $(MISC)$/$(@:s/_succeeded/.xml/:f) $(@:s/_succeeded/.xml/:f)
    $(TOUCH) $@
$(MISC)$/%_pdfi_unittest_draw_succeeded : $$(@:s/_succeeded/.xml/:f)
    rm -f $(MISC)$/$(@:s/_succeeded/.xml/:f)
    $(BIN)$/pdf2xml -draw $(@:s/_pdfi_unittest_draw_succeeded/.pdf/:f) $(MISC)$/$(@:s/_succeeded/.xml/:f) $(BIN)$/pdfi_unittest_test.ini
    diff $(MISC)$/$(@:s/_succeeded/.xml/:f) $(@:s/_succeeded/.xml/:f)
    $(TOUCH) $@

# hackaround for dmake's insufficiency to have more than one
# prerequisite on template rules
$(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_writer_succeeded/:f) $(MISC)$/$i)  : $(BIN)$/pdfi_unittest_test.ini $(BIN)$/pdf2xml$(EXECPOST)
$(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_impress_succeeded/:f) $(MISC)$/$i) : $(BIN)$/pdfi_unittest_test.ini $(BIN)$/pdf2xml$(EXECPOST)
$(foreach,i,$(TESTFILES:s/.pdf/_pdfi_unittest_draw_succeeded/:f) $(MISC)$/$i)    : $(BIN)$/pdfi_unittest_test.ini $(BIN)$/pdf2xml$(EXECPOST)

.ENDIF
