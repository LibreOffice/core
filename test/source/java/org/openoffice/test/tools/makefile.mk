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



PRJ = ../../../../../..
PRJNAME = test
TARGET = test-tools

PACKAGE = org/openoffice/test/tools

.INCLUDE: settings.mk

JARFILES = juh.jar ridl.jar unoil.jar
JAVAFILES = \
    OfficeDocument.java \
    OfficeDocumentView.java \
    DocumentType.java \
    SpreadsheetDocument.java \
    SpreadsheetView.java \

JARTARGET = $(TARGET).jar
JARCLASSDIRS = $(PACKAGE)
JARCLASSPATH = $(JARFILES)

.INCLUDE: target.mk

test:
    echo $(JAVACLASSFILES)
