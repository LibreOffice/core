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
# $Revision: 1.7.14.1 $
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


# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := test_climaker
PACKAGE = cliversion

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES = VersionTestCase.java
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



EXETARGET2=$(BIN)$/runtests.exe 

.IF "$(name)" != ""
TESTLIB=$(BIN)$/$(name)
.ELSE
TESTLIB=$(BIN)$/version_current.dll
.ENDIF
VERSIONLIBS=version_libs

.IF "$(GUI)"=="WNT"
ALLTAR: $(EXETARGET2) \
    $(TESTLIB) \
    COPYVERSIONLIBS \
    RUNINSTRUCTIONS
.ELSE
ALLTAR:
.ENDIF


COPYVERSIONLIBS: 
        -$(GNUCOPY) -p $(VERSIONLIBS)$/* $(BIN)

CSFILES2 = runtests.cs
$(EXETARGET2): $(CSFILES2)
    $(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET2) \
        $(CSFILES2)


CSFILESLIB = version.cs
$(TESTLIB): $(CSFILESLIB) $(BIN)$/cliureversion.mk
    $(CSC) $(CSCFLAGS) -target:library -out:$(TESTLIB) \
        -reference:$(BIN)$/cli_ure.dll \
         -reference:$(BIN)$/cli_types.dll \
         -reference:$(BIN)$/cli_basetypes.dll \
        -reference:$(BIN)$/cli_cppuhelper.dll \
        $(CSFILESLIB)

#This target only checks if the the office/program/assembly directory
#contains the proper libraries.
$(BIN)$/buildwithofficelibs.dll: 
    $(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:"$(office)"$/program$/assembly$/cli_ure.dll \
         -reference:"$(office)"$/program$/assembly$/cli_types.dll \
         -reference:"$(office)"$/program$/assembly$/cli_basetypes.dll \
        -reference:"$(office)"$/program$/assembly$/cli_cppuhelper.dll \
        $(CSFILESLIB)


#-----------------------------------------------------------------------------
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF


.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk


RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo To run the test you have to provide the path to the  office location.
    @echo Example:
    @echo dmake run office="d:\myOffice"
    @echo .
    @echo To build a test library with a particular name run. The namese must start with "version". 
    @echo For example:
    @echo "dmake name=version_10_10_10.dll"	
    @echo ###########################   N O T E  ######################################
    @echo .
    @echo .	

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN: $(BIN)$/buildwithofficelibs.dll
    java -cp $(CLASSPATH) -DSystemRoot=$(SystemRoot) -Dcli_test_program=$(EXETARGET2) -Duno_path="$(office)"\program $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN

