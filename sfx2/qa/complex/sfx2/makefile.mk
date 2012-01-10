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



.IF "$(OOO_JUNIT_JAR)" == ""
nothing .PHONY:
    @echo -----------------------------------------------------
    @echo - JUnit not available, not building anything
    @echo -----------------------------------------------------
.ELSE   # IF "$(OOO_JUNIT_JAR)" != ""

PRJ = ../../..
PRJNAME = sfx2
TARGET = qa_complex
PACKAGE = complex/sfx2

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = OOoRunnerLight.jar ridl.jar test.jar test-tools.jar unoil.jar
EXTRAJARFILES   = $(OOO_JUNIT_JAR)
JAVAFILES       = $(shell @$(FIND) . -name "*.java") \

#----- create a jar from compiled files ----------------------------

JARTARGET       = $(TARGET).jar

#----- JUnit tests class -------------------------------------------

JAVATESTFILES = \
    DocumentInfo.java \
    DocumentProperties.java \
    StandaloneDocumentInfo.java \
    DocumentMetadataAccess.java \
    UndoManager.java \

# disabled: #i115674#
#    GlobalEventBroadcaster.java \

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
