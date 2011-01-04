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
PRJNAME = bean
TARGET = qa_complex_bean

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/bean

# here store only Files which contain a @Test
JAVATESTFILES = \
    OOoBeanTest.java

# put here all other files
JAVAFILES = $(JAVATESTFILES) \
     ScreenComparer.java \
     WriterFrame.java

JARFILES = OOoRunner.jar ridl.jar test.jar unoil.jar officebean.jar
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
# 
# 
# PRJ := ..$/..
# PRJNAME := bean
# TARGET := test_bean
# PACKAGE = complex
# 
# .INCLUDE: settings.mk
# 
# #----- compile .java files -----------------------------------------
# 
# JARFILES = officebean.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
# JAVAFILES       = OOoBeanTest.java ScreenComparer.java WriterFrame.java
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
# # test base is java complex
# CT_TESTBASE = -TestBase java_complex
# 
# # test looks something like the.full.package.TestName
# CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)
# 
# # start the runner application
# CT_APP      = org.openoffice.Runner
# 
# CT_NOOFFICE = -NoOffice
# 
# 
# OFFICE_CLASSPATH_TMP:=$(foreach,i,$(JARFILES) $(office)$/program$/classes$/$(i)$(PATH_SEPERATOR))
# 
# OFFICE_CLASSPATH=$(OFFICE_CLASSPATH_TMP:t"")$(SOLARBINDIR)$/OOoRunner.jar$(PATH_SEPERATOR)$(CLASSDIR)
# 
# OOOBEAN_OPTIONS=-Dcom.sun.star.officebean.Options=-norestore -DOOoBean.Images=$(MISC)
# 
# 
# .INCLUDE: target.mk
# 
# ALLTAR : RUNINSTRUCTIONS
# 
# # --- Targets ------------------------------------------------------
# 
# #The OOoBean uses the classpath to find the office installation.
# #Therefore we must use the jar files from the office.
# RUN:
#     java -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)
# run: RUN
# 
# rund:
#     java -Xdebug -Xrunjdwp:transport=dt_socket,server=y,address=8100 -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)
# 
# 
# 
# RUNINSTRUCTIONS : 
#     @echo .
#     @echo ###########################   N O T E  ######################################
#     @echo . 
#     @echo "To run the test you have to provide the office location."
#     @echo Example:
#     @echo dmake run office="d:/myOffice"
#     @echo .
#   
