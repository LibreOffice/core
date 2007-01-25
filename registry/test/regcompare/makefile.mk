#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:39:07 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
