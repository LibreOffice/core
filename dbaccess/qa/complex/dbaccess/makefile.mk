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
PRJNAME = dbaccess
TARGET = qa_complex_dbaccess

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/dbaccess

# here store only Files which contain a @Test
JAVATESTFILES = \
    ApplicationController.java \
    Beamer.java \
    CRMBasedTestCase.java \
    CopyTableWizard.java \
    DataSource.java \
    DatabaseDocument.java \
    Parser.java \
    PropertyBag.java \
    Query.java \
    QueryInQuery.java \
    RowSet.java \
    SingleSelectQueryComposer.java \
    UISettings.java \
    TestCase.java 

# put here all other files
JAVAFILES = $(JAVATESTFILES) \
    CopyTableInterActionHandler.java \
    DatabaseApplication.java \
    FileHelper.java \
    RowSetEventListener.java 


JARFILES = OOoRunner.jar ridl.jar test.jar unoil.jar ConnectivityTools.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

# Sample how to debug
# JAVAIFLAGS=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : javatest

.END


# 
# 
# PRJ = ..$/..$/..
# TARGET  = DbaComplexTests
# PRJNAME = $(TARGET)
# PACKAGE = complex$/dbaccess
# 
# # --- Settings -----------------------------------------------------
# .INCLUDE: settings.mk
# 
# .IF "$(SOLAR_JAVA)" == ""
# all:
# 	@echo "Java not available. Build skipped"
# 
# .INCLUDE :  target.mk
# .ELSE
# 
# #----- compile .java files -----------------------------------------
# 
# JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar ConnectivityTools.jar
# JAVAFILES       := $(shell @$(FIND) ./*.java)
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
# RUNNER_ARGS = -cp "$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar" org.openoffice.Runner -TestBase java_complex 
# 
# RUNNER_CALL = $(AUGMENT_LIBRARY_PATH) java
# 
# # --- Targets ------------------------------------------------------
# 
# .IF "$(depend)" == ""
# ALL :   ALLTAR
# .ELSE
# ALL: 	ALLDEP
# .ENDIF
# 
# .INCLUDE :  target.mk
# 
# 
# run: $(CLASSDIR)$/$(JARTARGET)
#     +$(RUNNER_CALL) $(RUNNER_ARGS) -sce dbaccess.sce
# 
# run_%: $(CLASSDIR)$/$(JARTARGET)
#     +$(RUNNER_CALL) $(RUNNER_ARGS) -o complex.dbaccess.$(@:s/run_//)
# 
# .ENDIF # "$(SOLAR_JAVA)" == ""
