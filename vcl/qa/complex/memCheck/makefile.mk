#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
