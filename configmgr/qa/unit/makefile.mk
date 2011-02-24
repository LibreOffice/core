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
#***********************************************************************/

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
    $(SALLIB)

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
        '$${$$BRAND_BASE_DIR/program/bootstrap$(MY_INI):UserInstallation}' \
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
