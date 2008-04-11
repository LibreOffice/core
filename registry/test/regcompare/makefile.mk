#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.5 $
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
#*************************************************************************

PRJ := ..$/..
PRJNAME := registry
TARGET := test_regcompare

.INCLUDE: settings.mk
.INCLUDE: target.mk

ALLTAR: test

test .PHONY: $(MISC)$/$(TARGET)$/psa.urd $(MISC)$/$(TARGET)$/psb.urd \
        $(MISC)$/$(TARGET)$/usa.urd $(MISC)$/$(TARGET)$/usb.urd \
        $(MISC)$/$(TARGET)$/pe.urd $(MISC)$/$(TARGET)$/ue.urd \
        $(MISC)$/$(TARGET)$/other1.urd $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd

$(MISC)$/$(TARGET)$/%.urd: %.idl
    $(IDLC) -O$(MISC)$/$(TARGET) -cid -we $<
