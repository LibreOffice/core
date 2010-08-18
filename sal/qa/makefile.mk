#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=sal
TARGET=whole_sal_qa

# LLA: irrelevant
# ENABLE_EXCEPTIONS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# BEGIN ------------------------------------------------------------
# END --------------------------------------------------------------


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(runtests)"!=""
ALLTAR : test_all
.ENDIF

# OTHER STUFF ------------------------------------------------------
# test : test_all

# start tests with consideration of libs2test.txt with 'dmake test'
# run through all tests directories (from libs2test.txt) and try to start all tests
# use 'dmake test TESTOPT="buildall"'

# ALLTAR
test_all: 
        @echo ----------------------------------------------------------
        @echo - start sal unit tests
        @echo ----------------------------------------------------------
        $(PERL) buildall.pl $(TESTOPT)
