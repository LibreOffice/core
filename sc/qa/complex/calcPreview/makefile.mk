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
JARFILES = OOoRunner.jar ridl.jar test.jar unoil.jar jurt.jar
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
# JARFILES = mysql.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar mysql.jar
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
# CT_APPEXECCOMMAND = -AppExecutionCommand "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
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
