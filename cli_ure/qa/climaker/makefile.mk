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

PRJ := ..$/..
PRJNAME := cli_ure
TARGET := test_climaker
PACKAGE = climaker

#we use the climaker which is build by this project
CLIMAKER*=$(WRAPCMD) $(BIN)$/climaker
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = libreoffice.jar java_uno.jar OOoRunner.jar
JAVAFILES       = ClimakerTestCase.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE



CSCFLAGS = -incr
.IF "$(debug)" != ""
CSCFLAGS += -checked+ -define:DEBUG -define:TRACE -debug+
.ELSE
CSCFLAGS += -optimize+
.ENDIF


OUTDIR=$(BIN)$/qa$/climaker
EXETARGET=$(OUTDIR)$/test_climaker.exe

ALLTAR: $(EXETARGET)

CSFILES = climaker.cs testobjects.cs


$(EXETARGET): $(CSFILES) $(OUTDIR)$/cli_test_types.dll
    $(GNUCOPY) -p $(BIN)$/cli_cppuhelper.dll $(OUTDIR)$/cli_cppuhelper.dll
    $(GNUCOPY) -p $(BIN)$/cli_uretypes.dll $(OUTDIR)$/cli_uretypes.dll
    $(GNUCOPY) -p $(BIN)$/cli_basetypes.dll $(OUTDIR)$/cli_basetypes.dll
    $(GNUCOPY) -p $(BIN)$/cli_ure.dll $(OUTDIR)$/cli_ure.dll
    $(GNUCOPY) -p $(BIN)$/climaker.exe $(OUTDIR)
    $(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET) \
        -reference:$(BIN)$/cli_ure.dll \
         -reference:$(BIN)$/cli_uretypes.dll \
         -reference:$(BIN)$/cli_basetypes.dll \
        -reference:$(OUTDIR)$/cli_test_types.dll \
        $(CSFILES)



#-----------------------------------------------------------------------------
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF




$(OUTDIR)$/types.urd: types.idl
    - $(MKDIR) $(OUTDIR)
    $(IDLC) -O$(OUTDIR) -I$(SOLARIDLDIR) -cid -we $<

$(OUTDIR)$/types.rdb: $(OUTDIR)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(OUTDIR)$/cli_test_types.dll: $(OUTDIR)$/types.rdb $(BIN)$/climaker.exe $(BIN)$/cli_uretypes.dll
    $(CLIMAKER) $(CLIMAKERFLAGS) --out $@  \
        -r $(BIN)$/cli_uretypes.dll \
        -X $(SOLARBINDIR)$/types.rdb \
        $(OUTDIR)$/types.rdb



.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN:
    java -cp $(CLASSPATH) -Dcli_ure_test=$(EXETARGET) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN
