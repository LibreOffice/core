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
ABORT_ON_ASSERTION = TRUE

.INCLUDE: settings.mk

.INCLUDE: target.mk
.INCLUDE: installationtest.mk

.IF "$(depend)" == ""
# disable smoketest when cross-compiling for now; we can use wine at some stage (?)
.IF "$(CROSS_COMPILING)" != "YES"
ALLTAR : cpptest
.ELSE
ALLTAR : 
.END
.END

TEST_ARGUMENTS = smoketest.doc=$(OUTDIR)/bin/smoketestdoc.sxw
CPPTEST_LIBRARY = $(OUTDIR)/lib/$(DLLPRE)smoketest$(DLLPOST)

.IF "$(OS)" != "WNT" || "$(CROSS_COMPILING)" == "YES"
.IF "$(DISABLE_LINKOO)" == "TRUE"
my_linkoo =
.ELSE
my_linkoo = -l
.END
localinstall :
    $(RM) -r $(installationtest_instpath)
    $(MKDIRHIER) $(installationtest_instpath)
    ooinstall $(my_linkoo) $(installationtest_instpath)/opt
.IF "$(DISABLE_LINKOO)" == "TRUE"
    install-gdb-printers -L
.END
cpptest : localinstall
.END
