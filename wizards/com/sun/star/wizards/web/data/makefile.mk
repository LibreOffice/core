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



PRJ		= ..$/..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = web_data
PACKAGE = com$/sun$/star$/wizards$/web$/data

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

#.INCLUDE :  $(PRJ)$/util$/makefile.pmk
JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge commonwizards.jar 

.IF "$(SYSTEM_SAXON)" == "YES"
EXTRAJARFILES = $(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF

#JARCLASSDIRS	= com$/sun$/star$/wizards$/web
#JARTARGET		= $(TARGET).jar

# --- Files --------------------------------------------------------

JAVAFILES=			\
    CGArgument.java \
    CGContent.java \
    CGDesign.java \
    CGDocument.java \
    CGExporter.java \
    CGFilter.java \
    CGGeneralInfo.java \
    CGIconSet.java \
    CGImage.java \
    CGLayout.java \
    CGPublish.java \
    CGSession.java \
    CGSessionName.java \
    CGSettings.java \
    CGStyle.java \
    ConfigSetItem.java \
    TypeDetection.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
