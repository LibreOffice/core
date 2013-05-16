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


# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := qa_test_climaker
PACKAGE = cliversioning

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES = VersionTestCase.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE


.INCLUDE: target.mk

ALLTAR : \
    echo

echo :
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo To run the test you have to provide the path to the  office location. It must
    @echo contain the ure (d:\myOffice\LibreOffice\URE).
    @echo Also an office must be installed with full system integration.
    @echo Example:
    @echo dmake run office="d:\myOffice"
    @echo .
    @echo To build a test library with a particular name run. The names must start with "version". 
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

RUN: $(MISC)$/copyassemblies.done
    java -cp $(CLASSPATH) -DSystemRoot=$(SystemRoot) -Dcli_test_program=$(BIN)$/runtests.exe -Dpath="$(office)"\LibreOffice\URE\bin $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN

