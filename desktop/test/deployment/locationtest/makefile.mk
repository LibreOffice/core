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

PRJ		= ..$/..$/..
PRJNAME = location_test
PACKAGE = com$/sun$/star$/comp$/smoketest
TARGET  = com_sun_star_comp_smoketest

no_common_build_zip:=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES = ridl.jar jurt.jar unoil.jar juh.jar

JARTARGET = LocationTest.jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = MANIFEST.MF

ZIP1TARGET=locationtest
ZIP1LIST=*
ZIPFLAGS=-r
ZIP1DIR=$(MISC)$/$(TARGET)
ZIP1EXT=.oxt

# --- Files --------------------------------------------------------

COPY_OXT_MANIFEST:= $(MISC)$/$(TARGET)$/META-INF$/manifest.xml
JAVAFILES = LocationTest.java

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(JARTARGETN) : $(MISC)$/$(TARGET).javamaker.done

$(JAVACLASSFILES) : $(MISC)$/$(TARGET).javamaker.done

$(MISC)$/$(TARGET).javamaker.done: $(BIN)$/LocationTest.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(BIN)$/LocationTest.rdb: LocationTest.idl
    $(IDLC) -O$(MISC) -I$(SOLARIDLDIR) -cid -we $<
    +-$(RM) $@
    $(REGMERGE) $@ /UCR $(MISC)$/LocationTest.urd

$(MISC)$/$(ZIP1TARGET).createdir :
    +$(MKDIRHIER) $(MISC)$/$(TARGET)$/META-INF >& $(NULLDEV) && $(TOUCH) $@

$(MISC)$/$(TARGET)_resort : manifest.xml $(JARTARGETN) $(MISC)$/$(ZIP1TARGET).createdir $(BIN)$/LocationTest.rdb description.xml
    $(COPY)  manifest.xml $(MISC)$/$(TARGET)$/META-INF$/manifest.xml
    $(COPY)  $(JARTARGETN) $(MISC)$/$(TARGET)$/$(JARTARGET)
    $(COPY)  $(BIN)$/LocationTest.rdb $(MISC)$/$(TARGET)$/LocationTest.rdb
    $(COPY)  description.xml $(MISC)$/$(TARGET)$/description.xml	
    $(TOUCH) $@

$(ZIP1TARGETN) : $(MISC)$/$(TARGET)_resort $(MISC)$/$(ZIP1TARGET).createdir

