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

PRJ=..$/..$/..
PRJNAME=sdext
TARGET=xpdftest
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(ENABLE_PDFIMPORT)" != "TRUE"
@all:
    @echo "PDF Import extension disabled."
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/test_0_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_0_out.def text_0_out.def testinput.pdf
    $(BIN)$/xpdfimport -f $(MISC)$/binary_0_out testinput.pdf > $(MISC)$/text_0_out
    diff --strip-trailing-cr $(MISC)$/binary_0_out binary_0_out.def
    diff --strip-trailing-cr $(MISC)$/text_0_out text_0_out.def
    $(TOUCH) $@

$(MISC)$/test_1_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_1_out.def text_1_out.def book.pdf
    $(BIN)$/xpdfimport -f $(MISC)$/binary_1_out book.pdf > $(MISC)$/text_1_out
    diff --strip-trailing-cr $(MISC)$/binary_1_out binary_1_out.def
    diff --strip-trailing-cr $(MISC)$/text_1_out text_1_out.def
    $(TOUCH) $@
