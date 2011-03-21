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
PRJNAME=starmath
TARGET=qa_cppunit

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# --- Libs ---------------------------------------------------------

SHL1OBJS=  \
    $(SLO)/test_starmath.obj \
    $(SLO)/test_nodetotextvisitors.obj


SHL1STDLIBS= \
    $(CPPUNITLIB) \
    $(EDITENGLIB) \
    $(SVXCORELIB) \
    $(SVXLIB) \
    $(SFX2LIB) \
    $(XMLOFFLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB)   \
    $(I18NPAPERLIB)     \
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1LIBS=$(SLB)$/starmath.lib

SHL1TARGET= test_starmath
SHL1RPATH = NONE
SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF= $(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= version.map

# --- All object files ---------------------------------------------

SLOFILES= \
    $(SHL1OBJS) \


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

test_components = \
    component/framework/util/fwk \
    component/toolkit/util/tk \
    component/sfx2/util/sfx \
    configmgr \
    vcl \
    mcnttype \
    i18npool
.IF "$(OS)" == "WNT"
test_components += \
    sysdtrans \
    ftransl
.ENDIF

# Make a services.rdb with the services we know we need to get up and running
$(MISC)/services.input : makefile.mk
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(test_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : makefile.mk $(MISC)/services.input
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@.tmp \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/services.input
    cat $(MISC)/$@.tmp | sed 's|/program/|/|g' > $@

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN)$(PATH_SEPERATOR)$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) -headless -invisible \
        '-env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb $(my_file)$(SOLARBINDIR)$/types.rdb' \
        '-env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb'\
        -env:URE_INTERNAL_LIB_DIR="$(my_file)$(SOLARSHAREDBIN)" \
        -env:OOO_BASE_DIR="$(my_file)$(SOLARSHAREDBIN)"
