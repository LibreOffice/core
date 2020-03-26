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

.IF "$(OOO_JUNIT_JAR)" == ""
nothing .PHONY:
    @echo -----------------------------------------------------
    @echo - JUnit not available, not building anything
    @echo -----------------------------------------------------
.ELSE

PRJ = ../../..
PRJNAME = dbaccess
TARGET = qa_complex_dbaccess
PACKAGE = complex/dbaccess

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = OOoRunner.jar libreoffice.jar test.jar ConnectivityTools.jar
EXTRAJARFILES   = $(OOO_JUNIT_JAR)

#----- create a jar from compiled files ----------------------------

JARTARGET       = $(TARGET).jar

#----- Java files --------------------------------------------------

# here store only Files which contain a @Test
JAVATESTFILES = \
    ApplicationController.java \
    Beamer.java \
    DataSource.java \
    DatabaseDocument.java \
    Parser.java \
    PropertyBag.java \
    Query.java \
    QueryInQuery.java \
    RowSet.java \
    SingleSelectQueryComposer.java \
    UISettings.java \
    CopyTableWizard.java \

# put here all other files
JAVAFILES = $(JAVATESTFILES) \
    CRMBasedTestCase.java \
    CopyTableInterActionHandler.java \
    DatabaseApplication.java \
    FileHelper.java \
    RowSetEventListener.java \
    TestCase.java \


# Sample how to debug
# JAVAIFLAGS=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

# --- Targets ------------------------------------------------------

.INCLUDE: target.mk

ALL :   ALLTAR

# --- subsequent tests ---------------------------------------------

.IF "$(OOO_SUBSEQUENT_TESTS)" != ""

.INCLUDE: installationtest.mk

ALLTAR : javatest

    # Sample how to debug
    # JAVAIFLAGS=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

.END    # "$(OOO_SUBSEQUENT_TESTS)" == ""

.END    # ELSE "$(OOO_JUNIT_JAR)" != ""

