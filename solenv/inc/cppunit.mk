#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cppunit.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:07:55 $
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
# $(SHL$(TNR)TARGETN) is the same like $(OUT)$/bin$/$(SHL1TARGET) for every number, within unix, "bin" is replaced by "lib"

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
