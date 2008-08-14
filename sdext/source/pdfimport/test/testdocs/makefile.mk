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
# $Revision: 1.2 $
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
.IF "$(ENABLE_PDFIMPORT)" != "NO"

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
