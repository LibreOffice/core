#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-04-10 09:09:31 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
