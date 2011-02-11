#*************************************************************************
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
#***********************************************************************/

PRJ = .
PRJNAME = smoketestoo_native
TARGET = smoketest

ENABLE_EXCEPTIONS = TRUE

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

TEST_ARGUMENTS = smoketest.doc=$(OUTDIR)/bin$(PROEXT)/smoketestdoc.sxw
CPPTEST_LIBRARY = $(SHL1TARGETN)

.IF "$(OS)" != "WNT"
$(installationtest_instpath).flag : $(shell ls \
        $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).tar.gz)
    $(RM) -r $(installationtest_instpath)
    $(MKDIRHIER) $(installationtest_instpath)
    cd $(installationtest_instpath) && $(GNUTAR) xfz \
        $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).tar.gz
    $(MV) $(installationtest_instpath)/OOo_*_install-arc_$(defaultlangiso) \
        $(installationtest_instpath)/opt
    $(TOUCH) $@
cpptest : $(installationtest_instpath).flag
.END
