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
# $Revision: 1.8.8.1 $
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
PRJNAME := cli_ure
TARGET := test_climaker
PACKAGE = climaker

#we use the climaker which is build by this project
CLIMAKER*=$(WRAPCMD) $(BIN)$/climaker
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
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
