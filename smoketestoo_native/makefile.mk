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



PRJ = .
PRJNAME = smoketestoo_native
TARGET = smoketest

ENABLE_EXCEPTIONS = TRUE

ABORT_ON_ASSERTION = TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

SLOFILES = $(SHL1OBJS)

SHL1TARGET = smoketest
SHL1OBJS = $(SLO)/smoketest.obj
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(CPPUNITLIB) $(SALLIB) $(TESTLIB)
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : cpptest

cpptest : $(SHL1TARGETN)

TEST_ARGUMENTS = smoketest.doc=$(OUTDIR)/bin$(UPDMINOREXT)/smoketestdoc.sxw
CPPTEST_LIBRARY = $(SHL1TARGETN)

.IF "$(OS)" != "WNT"
$(installationtest_instpath).flag : $(shell ls \
        $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).tar.gz)
    $(COMMAND_ECHO)$(RM) -r $(installationtest_instpath)
    $(COMMAND_ECHO)$(MKDIRHIER) $(installationtest_instpath)
    $(COMMAND_ECHO)cd $(installationtest_instpath) && $(GNUTAR) xfz \
        $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).tar.gz
    $(COMMAND_ECHO)$(MV) $(installationtest_instpath)/OOo_*_install-arc_$(defaultlangiso) \
        $(installationtest_instpath)/opt
    $(COMMAND_ECHO)$(TOUCH) $@
cpptest : $(installationtest_instpath).flag
.END
