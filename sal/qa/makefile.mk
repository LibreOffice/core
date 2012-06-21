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
