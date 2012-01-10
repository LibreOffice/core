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

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar
JAVAFILES = \
 TriState.java \
 IniFile.java \
 BuildID.java \
 GfxCompare.java \
 FilenameHelper.java \
 NameHelper.java \
 HTMLOutputter.java \
 LISTOutputter.java \
 INIOutputter.java \
 PropertyName.java \
 StatusHelper.java \
 ConvWatchException.java \
 ConvWatchCancelException.java \
 OfficePrint.java \
 PRNCompare.java \
 FileHelper.java \
 OSHelper.java \
 PixelCounter.java \
 ImageHelper.java \
 BorderRemover.java \
 ConvWatch.java \
 DirectoryHelper.java \
 ConvWatchStarter.java \
 ReferenceBuilder.java \
 EnhancedComplexTestCase.java \
 MSOfficePrint.java \
 GraphicalTestArguments.java \
 GraphicalDifferenceCheck.java \
 DocumentConverter.java\
 DBHelper.java\
 DB.java\
 ValueNotFoundException.java \
 GlobalLogWriter.java \
 CrashLoopTest.java \
 ReportDesignerTest.java \
 DateHelper.java \
 TimeHelper.java




JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
