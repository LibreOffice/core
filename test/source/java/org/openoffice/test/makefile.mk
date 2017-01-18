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



PRJ = ../../../../..
PRJNAME = test
TARGET = test

.IF "$(OOO_JUNIT_JAR)" != ""

PACKAGE = org/openoffice/test
JAVAFILES = \
    Argument.java \
    FileHelper.java \
    OfficeConnection.java \
    OfficeFileUrl.java \
    TestArgument.java
JARFILES = juh.jar ridl.jar unoil.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

JARTARGET        = $(TARGET).jar
JARCLASSDIRS     = $(PACKAGE)
JARCLASSEXCLUDES = $(PACKAGE)/tools/*
JARCLASSPATH     = $(JARFILES)
 # expect $(OOO_JUNIT_JAR) to be on CLASSPATH wherever test.jar is used (also,
 # on Windows, $(OOO_JUNIT_JAR) could be an absolute pathname with drive letter
 # like X:/path and some JVMs would refuse to load test.jar if its MANIFEST.MF
 # Class-Path contained such a pathname that looks like an unknown URL with
 # scheme X)

.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
