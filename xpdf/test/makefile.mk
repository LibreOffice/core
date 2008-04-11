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
# $Revision: 1.3 $
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
PRJNAME=xpdf
TARGET=test
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

#ALLTAR : $(MISC)$/test_0_succeeded $(MISC)$/test_1_succeeded

$(MISC)$/test_0_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_0_out.def text_0_out.def testinput.pdf
    $(BIN)$/xpdfimport -f $(MISC)$/binary_0_out testinput.pdf > $(MISC)$/text_0_out
    diff $(MISC)$/binary_0_out binary_0_out.def
    diff $(MISC)$/text_0_out text_0_out.def
    $(TOUCH) $@

$(MISC)$/test_1_succeeded: $(BIN)$/xpdfimport$(EXECPOST) binary_1_out.def text_1_out.def book.pdf
    $(BIN)$/xpdfimport -f $(MISC)$/binary_1_out book.pdf > $(MISC)$/text_1_out
    diff $(MISC)$/binary_1_out binary_1_out.def
    diff $(MISC)$/text_1_out text_1_out.def
    $(TOUCH) $@
