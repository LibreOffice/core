#*************************************************************************
#
#   $RCSfile: cppunit.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2003-11-18 16:07:34 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

.IF "$(TESTOPT)"==""
    TESTOPT="-onlyerrors"
.ENDIF

################################################################################
# unroll begin
# ---- create the test libraries ------------------------------------

TEST$(TNR)LIB=$(SHL$(TNR)TARGETN)

.IF "$(SHL$(TNR)TARGET)"!=""
test : test$(TNR)
test$(TNR): ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#$(TNR) on library $(TEST$(TNR)LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST$(TNR)LIB) $(TESTOPT)
.ENDIF

# unroll end
