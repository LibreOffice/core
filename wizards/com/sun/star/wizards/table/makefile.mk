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
TARGET  = table
PACKAGE = com$/sun$/star$/wizards$/table

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

JARFILES= unoil.jar jurt.jar ridl.jar juh.jar java_uno.jar commonwizards.jar
CUSTOMMANIFESTFILE= MANIFEST.MF

JARCLASSDIRS	= com$/sun$/star$/wizards$/table
JARTARGET		= $(TARGET).jar
JARCLASSPATH = commonwizards.jar

# --- Files --------------------------------------------------------
#SRS2NAME =              dbwizres
#SRC2FILES=		dbwizres.src
#RESLIB2SRSFILES= $(SRS)$/dbwizres.srs
#RESLIB2NAME=	dbw

JAVAFILES=	\
        CallTableWizard.java 	\
        Finalizer.java 		\
        FieldDescription.java   \
        CGCategory.java 	\
        CGTable.java		\
        FieldFormatter.java	\
        PrimaryKeyHandler.java	\
        ScenarioSelector.java	\
        TableWizard.java	\

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/table.component

$(MISC)/table.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        table.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt table.component
