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



# Helper makefile additional, to support
# dmake test
# for cppunittester (cppunit testing)
# for simple start all generated libraries:
# dmake test
#
# for library 1 & 2:
# or dmake test1 test2

# This file have to be unroll by
# cat cppunit.mk | mkunroll >_cppunit.mk
# all between begin and end will repeat n times (at the moment n=10) to create such iterations
# $(TNR) is the number
# $(SHL$(TNR)TARGETN) is the same like $(OUT)/bin/$(SHL1TARGET) for every number, within unix, "bin" is replaced by "lib"

################################################################################
# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL$(TNR)TARGETN)"!=""
ALLTAR : test$(TNR)
test$(TNR) .PHONY : $(SHL$(TNR)TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#$(TNR) on library $(SHL$(TNR)TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL$(TNR)TARGETN)
.ENDIF

# unroll end
