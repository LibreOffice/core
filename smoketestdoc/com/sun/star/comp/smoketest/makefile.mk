#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
