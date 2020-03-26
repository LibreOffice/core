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

.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ = ../../..
PRJNAME = sc
TARGET = qa_complex_calcpreview

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/calcPreview
JAVATESTFILES = \
    TestDocument.java \
    ViewForwarder.java

JAVAFILES = $(JAVATESTFILES)
JARFILES = OOoRunner.jar libreoffice.jar test.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)
.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : javatest

.END




# PRJ = ..$/..$/..
# TARGET  = ViewForward
# PRJNAME = $(TARGET)
# PACKAGE = complex$/calcPreview
#
# # --- Settings -----------------------------------------------------
# .INCLUDE: settings.mk
#
#
# #----- compile .java files -----------------------------------------
#
# JARFILES = mysql.jar libreoffice.jar java_uno.jar OOoRunner.jar mysql.jar
# JAVAFILES       = ViewForwarder.java
# JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)
#
# #----- make a jar from compiled files ------------------------------
#
# MAXLINELENGTH = 100000
#
# JARCLASSDIRS    = $(PACKAGE)
# JARTARGET       = $(TARGET).jar
# JARCOMPRESS 	= TRUE
#
# # --- Parameters for the test --------------------------------------
#
# # start an office if the parameter is set for the makefile
# .IF "$(OFFICE)" == ""
# CT_APPEXECCOMMAND =
# .ELSE
# CT_APPEXECCOMMAND = -AppExecutionCommand "$(OFFICE)$/soffice --accept=socket,host=localhost,port=8100;urp;"
# .ENDIF
#
# # test base is java complex
# CT_TESTBASE = -TestBase java_complex
#
# # set test document path
# CT_TESTDOCS = -tdoc $(PWD)$/test_documents
#
# # test looks something like the.full.package.TestName
# CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)
#
# # start the runner application
# CT_APP      = org.openoffice.Runner
#
# # set the timeout to a bigger value
# CT_TIMEOUT = -TimeOut 120000
#
# # --- Targets ------------------------------------------------------
#
# .IF "$(depend)" == ""
# $(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props : ALLTAR
# .ELSE
# $(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props : ALLDEP
# .ENDIF
#
# .INCLUDE :  target.mk
#
#
# RUN:
#     +java -cp "$(CLASSPATH)" $(CT_APP) $(CT_APPEXECCOMMAND) $(CT_TESTDOCS) $(CT_TESTBASE) $(CT_WORKDIR) $(CT_TIMEOUT) $(CT_TEST)
#
# run: RUN
#
# tst:
# 	+@echo $(CT_TESTDOCS)
#
