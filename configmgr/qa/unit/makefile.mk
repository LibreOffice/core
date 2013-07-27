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



PRJ = ../..
PRJNAME = configmgr
TARGET = unit

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SLOFILES = $(SLO)/test.obj

SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(CPPUNITLIB) \
    $(SALLIB) \
    $(TESTSHL2LIB)
SHL1TARGET = unit
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR: TEST

.IF "$(OS)" == "OS2" || "$(OS)" == "WNT"
MY_INI = .ini
.ELSE
MY_INI = rc
.ENDIF

$(MISC)/unit.rdb .ERRREMOVE:
    cp $(SOLARBINDIR)/types.rdb $@
    $(REGCOMP) -register -r $@ -c $(DLLDEST)/$(DLLPRE)configmgr$(DLLPOST)

TEST .PHONY: $(SHL1TARGETN) $(MISC)/unit.rdb
    rm -rf $(MISC)/unitdata
    mkdir $(MISC)/unitdata
    cp urebootstrap.ini $(MISC)/unitdata
    mkdir $(MISC)/unitdata/basis
    mkdir $(MISC)/unitdata/basis/program
    echo '[Bootstrap]' > $(MISC)/unitdata/basis/program/uno$(MY_INI)
    echo 'UNO_SHARED_PACKAGES_CACHE = $$OOO_BASE_DIR' \
        >> $(MISC)/unitdata/basis/program/uno$(MY_INI)
    echo 'UNO_USER_PACKAGES_CACHE =' \
        '$${$$OOO_BASE_DIR/program/bootstrap$(MY_INI):UserInstallation}' \
        >> $(MISC)/unitdata/basis/program/uno$(MY_INI)
    mkdir $(MISC)/unitdata/basis/share
    mkdir $(MISC)/unitdata/basis/share/registry
    cp data.xcd $(MISC)/unitdata/basis/share/registry
    mkdir $(MISC)/unitdata/brand
    mkdir $(MISC)/unitdata/brand/program
    echo '[Bootstrap]' > $(MISC)/unitdata/brand/program/bootstrap$(MY_INI)
    echo 'UserInstallation = $$ORIGIN/../../user' \
        >> $(MISC)/unitdata/brand/program/bootstrap$(MY_INI)
.IF "$(USE_SHELL)" == "bash"
    export \
        URE_BOOTSTRAP=vnd.sun.star.pathname:$(MISC)/unitdata/urebootstrap.ini \
        && $(TESTSHL2) $(SHL1TARGETN) -forward $(MISC)/unit.rdb
.ELSE
    setenv \
        URE_BOOTSTRAP vnd.sun.star.pathname:$(MISC)/unitdata/urebootstrap.ini \
        && $(TESTSHL2) $(SHL1TARGETN) -forward $(MISC)/unit.rdb
.ENDIF
