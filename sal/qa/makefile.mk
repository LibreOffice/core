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
