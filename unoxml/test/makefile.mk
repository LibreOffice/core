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

PRJNAME=unoxml
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.
 
.ELSE

# --- Common ----------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:tests by codegen.pl
APP1OBJS=  \
    $(SLO)$/domtest.obj $(SLO)$/documentbuilder.obj

# the following three libs are a bit of a hack: cannot link against
# unoxml here, because not yet delivered (and does not export
# ~anything). Need the functionality to test, so we're linking it in
# statically. Need to keep this in sync with
# source/services/makefile.mk
#APP1LIBS= \
    #$(SLB)$/domimpl.lib \
    #$(SLB)$/xpathimpl.lib \
    #$(SLB)$/eventsimpl.lib

APP1TARGET= tests
APP1STDLIBS= \
    $(UCBHELPERLIB) \
    $(LIBXML2LIB) \
    $(TOOLSLIB)	\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(GTESTLIB)	 \
        $(TESTSHL2LIB)    \
    $(CPPULIB)	\
    $(SAXLIB) \
    $(SALLIB)\
    $(EXPATASCII3RDLIB) \
    -lunoxml

APP1RPATH = NONE
APP1TEST = disabled

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(APP1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

# --- Fake uno bootstrap ------------------------

$(BIN)$/unoxml_unittest_test.ini : makefile.mk
    rm -f $@
    @echo UNO_SERVICES= > $@
    @echo UNO_TYPES=$(UNOUCRRDB:s/\/\\/) >> $@

# --- Enable testshl2 execution in normal build ------------------------

$(MISC)$/unoxml_unittest_succeeded : $(SHL1TARGETN) $(BIN)$/unoxml_unittest_test.ini
        @echo ----------------------------------------------------------
        @echo - start unit test on library $(SHL1TARGETN)
        @echo ----------------------------------------------------------
        $(COMMAND_ECHO) $(AUGMENT_LIBRARY_PATH_LOCAL) \
        UNOXML_DOMTEST_FORWARD=$(BIN)$/ \
        $(APP1TARGETN) --gtest_output="xml:$(BIN)/$(APP1TARGET)_result.xml"
        $(TOUCH) $@

ALLTAR : $(MISC)$/unoxml_unittest_succeeded

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
