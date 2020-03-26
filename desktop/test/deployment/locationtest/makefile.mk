#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ		= ..$/..$/..
PRJNAME = location_test
PACKAGE = com$/sun$/star$/comp$/smoketest
TARGET  = com_sun_star_comp_smoketest

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES = libreoffice.jar

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

.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN) : $(MISC)$/$(TARGET)_resort $(MISC)$/$(ZIP1TARGET).createdir

.ENDIF          # "$(ZIP1TARGETN)"!=""

