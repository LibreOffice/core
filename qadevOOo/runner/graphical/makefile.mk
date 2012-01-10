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


PRJ=..$/..

PRJNAME = OOoRunner
PACKAGE = convwatch
TARGET = runner_convwatch

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar
JAVAFILES = \
BuildID.java \
DateHelper.java \
DirectoryHelper.java \
EnhancedComplexTestCase.java \
FileHelper.java \
GlobalLogWriter.java \
HTMLResult.java \
IDocument.java \
IOffice.java \
ImageHelper.java \
IniFile.java \
JPEGComparator.java \
JPEGCreator.java \
JPEGEvaluator.java \
MSOfficePostscriptCreator.java \
Office.java \
OfficeException.java \
OpenOfficeDatabaseReportExtractor.java \
OpenOfficePostscriptCreator.java \
ParameterHelper.java \
PerformanceContainer.java \
PixelCounter.java \
PostscriptCreator.java \
PropertyName.java \
TimeHelper.java \
WrongEnvironmentException.java \
WrongSuffixException.java \
Tolerance.java

# GraphicalComparator.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
