#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: cppunit.mk,v $
#
# $Revision: 1.9 $
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

# Helper makefile additional, to support
# dmake test
# for testshl2 (cppunit testing)
# for simple start all generated libraries:
# dmake test
#
# for library 1 & 2:
# or dmake test1 test2
#
# for library 2 with additional options to the testshl2
# dmake test2 TESTOPT="-dntsf"

# This file have to be unroll by
# cat cppunit.mk | mkunroll >_cppunit.mk
# all between begin and end will repeat n times (at the moment n=10) to create such iterations
# $(TNR) is the number
# $(SHL$(TNR)TARGETN) is the same like $(OUT)/bin/$(SHL1TARGET) for every number, within unix, "bin" is replaced by "lib"

#.IF "$(TESTOPT)"==""
#	TESTOPT="-onlyerrors"
#.ENDIF

################################################################################
# unroll begin
# ---- create the test libraries ------------------------------------

TEST$(TNR)LIB=$(SHL$(TNR)TARGETN)
.IF "$(TESTOPT)"==""
    TEST$(TNR)OPT="-jobexclude"
    TEST$(TNR)OPT+=$(SHL$(TNR)TARGET).xsce
    TEST$(TNR)OPT+="-sf"
    TEST$(TNR)OPT+=$(mktmp "foo")
    TEST$(TNR)OPT+="-onlyerrors"
    TEST$(TNR)OPT+=" "
    TEST$(TNR)OPT+=$(TESTOPTADD)
.ELSE
    TEST$(TNR)OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL$(TNR)TARGET)"!=""
test : test$(TNR)
test$(TNR): ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#$(TNR) on library $(TEST$(TNR)LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST$(TNR)LIB) $(TEST$(TNR)OPT)
.ELSE
        testshl2 $(TEST$(TNR)LIB) $(TEST$(TNR)OPT)
.ENDIF
.ENDIF

# unroll end
