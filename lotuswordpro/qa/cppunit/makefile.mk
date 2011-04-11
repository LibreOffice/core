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
#*************************************************************************

PRJ=../..
PRJNAME=lotuswordpro
TARGET=qa_lotuswordpro

ENABLE_EXCEPTIONS=TRUE

my_components = qa_lwpfilter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)
DLLPRE = # no leading "lib" on .so files

# --- Libs ---------------------------------------------------------

SHL1OBJS=  \
    $(SLO)/test_lotuswordpro.obj

SHL1STDLIBS= \
     $(VCLLIB) \
     $(COMPHELPERLIB) \
     $(CPPUHELPERLIB) \
     $(CPPULIB) \
     $(SALLIB) \
     $(CPPUNITLIB)

SHL1TARGET= test_lotuswordpro
SHL1RPATH = NONE
SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP= version.map

# --- All object files ---------------------------------------------

SLOFILES= \
    $(SHL1OBJS)


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

$(MISC)$/$(TARGET)$/types.rdb .ERRREMOVE : $(SOLARBINDIR)$/types.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

$(MISC)/$(TARGET)/udkapi.rdb .ERRREMOVE : $(SOLARBINDIR)$/udkapi.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

#Make a services.rdb with the services we know we need to get up and running
$(MISC)/$(TARGET)/services.rdb .ERRREMOVE: $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/services.input \
        $(my_components:^"$(MISC)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/services.input

$(MISC)/$(TARGET)/services.input:
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb $(MISC)$/$(TARGET)$/types.rdb $(MISC)/$(TARGET)/udkapi.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) --headless --invisible \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES="$(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/udkapi.rdb" \
        -env:OOO_INBUILD_SHAREDLIB_DIR="$(my_file)$(PWD)/$(DLLDEST)"
