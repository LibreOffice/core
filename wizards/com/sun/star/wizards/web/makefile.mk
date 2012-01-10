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



PRJ		= ..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = web
PACKAGE = com$/sun$/star$/wizards$/web

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk
JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge commonwizards.jar 

.IF "$(SYSTEM_SAXON)" == "YES"
EXTRAJARFILES = $(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF

CUSTOMMANIFESTFILE= MANIFEST.MF

JARCLASSDIRS	= com$/sun$/star$/wizards$/web
JARTARGET		= web.jar
JARCLASSPATH = \
    commonwizards.jar saxon9.jar

# --- Files --------------------------------------------------------

JAVAFILES=			\
    AbstractErrorHandler.java \
    BackgroundsDialog.java \
    CallWizard.java \
    ErrorHandler.java \
    ExtensionVerifier.java \
    FTPDialog.java \
    FTPDialogResources.java \
    IconsDialog.java \
    ImageListDialog.java \
    LogTaskListener.java \
    Process.java \
    ProcessErrorHandler.java \
    ProcessErrors.java \
    ProcessStatusRenderer.java \
    StatusDialog.java \
    StylePreview.java \
    TOCPreview.java \
    WebWizard.java \
    WebWizardConst.java \
    WebWizardDialog.java \
    WebWizardDialogResources.java \
    WWD_Events.java \
    WWD_General.java \
    WWD_Startup.java \
    WWHID.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/web.component

$(MISC)/web.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        web.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt web.component
