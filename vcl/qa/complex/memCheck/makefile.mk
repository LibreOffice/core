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
PRJNAME = vcl
TARGET = qa_complex_memCheck

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = complex/memCheck

# here store only Files which contain a @Test
JAVATESTFILES = \
    CheckMemoryUsage.java

# put here all other files
JAVAFILES = $(JAVATESTFILES) \
 FileHelper.java \
 TestDocument.java


JARFILES = OOoRunner.jar ridl.jar test.jar unoil.jar
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
# PRJ = ..$/..$/..
# TARGET  = MemoryCheck
# PRJNAME = $(TARGET)
# PACKAGE = complex$/memCheck
# 
# # --- Settings -----------------------------------------------------
# .INCLUDE: settings.mk
# 
# 
# #----- compile .java files -----------------------------------------
# 
# JARFILES  = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
# JAVAFILES = CheckMemoryUsage.java 
# 
# #----- make a jar from compiled files ------------------------------
# 
# MAXLINELENGTH = 100000
# 
# JARCLASSDIRS  = $(PACKAGE)
# JARTARGET     = $(TARGET).jar
# JARCOMPRESS   = TRUE
# 
# # --- Parameters for the test --------------------------------------
# 
# # start an office if the parameter is set for the makefile
# .IF "$(OFFICE)" == ""
# CT_APPEXECCOMMAND =
# .ELSE
# CT_APPEXECCOMMAND = -AppExecutionCommand \
#             "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
# .ENDIF
# 
# # test base is java complex
# CT_TESTBASE = -TestBase java_complex
# 
# # replace $/ with . in package name
# CT_PACKAGE  = -o $(PACKAGE:s\$/\.\)
# 
# # start the runner application
# CT_APP      = org.openoffice.Runner
# 
# # --- Targets ------------------------------------------------------
# 
# .IF "$(depend)" == ""
# $(CLASSDIR)$/$(PACKAGE)$/CheckMemoryUsage.props : ALLTAR
# .ELSE
# $(CLASSDIR)$/$(PACKAGE)$/CheckMemoryUsage.props : ALLTAR
# .ENDIF
# 
# .INCLUDE :  target.mk
# 
# 
# 
# $(CLASSDIR)$/$(PACKAGE)$/CheckMemoryUsage.props : CheckMemoryUsage.props
#     cp $(@:f) $@
#     jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/$(@:f)
# 
# 
# RUN: run
# 
# run: 
#     java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_PACKAGE).CheckMemoryUsage
