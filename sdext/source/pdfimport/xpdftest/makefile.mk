#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..
PRJNAME=sdext
TARGET=xpdftest
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(ENABLE_PDFIMPORT)" == "NO"
@all:
    @echo "PDF Import extension disabled."
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

#ALLTAR : $(MISC)$/test_0_succeeded

$(MISC)$/test_0_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_0_out.def text_0_out.def testinput.pdf
    $(BIN)$/xpdfimport -f $(MISC)$/binary_0_out testinput.pdf > $(MISC)$/text_0_out
    diff --strip-trailing-cr $(MISC)$/binary_0_out binary_0_out.def
    diff --strip-trailing-cr $(MISC)$/text_0_out text_0_out.def
    $(TOUCH) $@

# This test is disabled because
# - the PDF import extension is not part of the AOO build #i118592#
# - the test files are too huge (75MB + 5MB + 500kB) to remain in the tree
# The test files are still available at
#   http://svn.apache.org/repos/asf/!svn/bc/1500000/openoffice/tags/AOO341/main/sdext/source/pdfimport/xpdftest/binary_1_out.def
#   http://svn.apache.org/repos/asf/!svn/bc/1500000/openoffice/tags/AOO341/main/sdext/source/pdfimport/xpdftest/text_1_out.def
#   http://svn.apache.org/repos/asf/!svn/bc/1500000/openoffice/tags/AOO341/main/sdext/source/pdfimport/xpdftest/book.pdf
# if you download them you can enable the test below:

#$(MISC)$/test_1_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_1_out.def text_1_out.def book.pdf
#	$(BIN)$/xpdfimport -f $(MISC)$/binary_1_out book.pdf > $(MISC)$/text_1_out
#	diff --strip-trailing-cr $(MISC)$/binary_1_out binary_1_out.def
#	diff --strip-trailing-cr $(MISC)$/text_1_out text_1_out.def
#	$(TOUCH) $@

