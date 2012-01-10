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
PRJNAME=odk
TARGET=cli

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

CSHARP=\
    $(DESTDIRCLIEXAMPLES)$/CSharp$/Spreadsheet$/GeneralTableSample.cs \
    $(DESTDIRCLIEXAMPLES)$/CSharp$/Spreadsheet$/SpreadsheetDocHelper.cs \
    $(DESTDIRCLIEXAMPLES)$/CSharp$/Spreadsheet$/SpreadsheetSample.cs \
    $(DESTDIRCLIEXAMPLES)$/CSharp$/Spreadsheet$/ViewSample.cs \
    $(DESTDIRCLIEXAMPLES)$/CSharp$/Spreadsheet$/Makefile

VBDOTNET=\
    $(DESTDIRCLIEXAMPLES)$/VB.NET$/WriterDemo$/WriterDemo.vb \
    $(DESTDIRCLIEXAMPLES)$/VB.NET$/WriterDemo$/Makefile


DIR_FILE_LIST= \
    $(CSHARP) \
    $(VBDOTNET)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/ex_$(TARGET)_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/ex_$(TARGET)_files.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk

