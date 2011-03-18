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

PRJ     = ../../../../..
PRJNAME = smoketestdoc
PACKAGE = com/sun/star/comp/smoketest
TARGET  = com_sun_star_comp_smoketest

no_common_build_zip:=TRUE

# --- Settings -----------------------------------------------------

.IF "$(SOLAR_JAVA)" != ""

.INCLUDE : settings.mk

JARFILES = ridl.jar jurt.jar unoil.jar juh.jar

JARTARGET          = TestExtension.jar
JARCOMPRESS        = TRUE
CUSTOMMANIFESTFILE = MANIFEST.MF

ZIP1TARGET=TestExtension
ZIP1LIST=*
ZIPFLAGS=-r
ZIP1DIR=$(MISC)/$(TARGET)
ZIP1EXT=.oxt

# --- Files --------------------------------------------------------

#COPY_OXT_MANIFEST:= $(MISC)/$(TARGET)/META-INF/manifest.xml
JAVAFILES = TestExtension.java

# --- Targets ------------------------------------------------------

ZIP1DEPS=$(MISC)/$(TARGET)/$(JARTARGET)\
         $(MISC)/$(TARGET)/TestExtension.rdb\
         $(MISC)/$(TARGET)/META-INF/manifest.xml

.INCLUDE :  target.mk

$(MISC)/$(TARGET)/META-INF:
    @-$(MKDIRHIER) $(@)

$(MISC)/TestExtension.urd: TestExtension.idl
    $(COMMAND_ECHO)$(IDLC) -O$(MISC) -I$(SOLARIDLDIR) -cid -we $<

$(MISC)/$(TARGET)/META-INF/manifest.xml $(MISC)/$(TARGET)/TestExtension.rdb : $(MISC)/$(TARGET)/META-INF

$(MISC)/$(TARGET)/META-INF/manifest.xml: manifest.xml
    $(COMMAND_ECHO)$(COPY) $< $@

$(MISC)/$(TARGET)/TestExtension.rdb: $(MISC)/TestExtension.urd
    $(COMMAND_ECHO)$(REGMERGE) $@ /UCR $<

$(JAVACLASSFILES) .UPDATEALL: $(MISC)/$(TARGET)/TestExtension.rdb
    $(COMMAND_ECHO)$(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<

$(MISC)/$(TARGET)/$(JARTARGET) : $(JARTARGETN)
    $(COMMAND_ECHO)$(COPY) $< $@

.ELSE
@all:
    @echo "Java disabled. Nothing to do."
.ENDIF
