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
TARGET  = report
PACKAGE = com$/sun$/star$/wizards$/report

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

JARFILES= unoil.jar jurt.jar ridl.jar juh.jar java_uno.jar commonwizards.jar

CUSTOMMANIFESTFILE= MANIFEST.MF

JARCLASSDIRS	= com$/sun$/star$/wizards$/report
JARTARGET	= $(TARGET).jar
JARCLASSPATH = commonwizards.jar

# --- Files --------------------------------------------------------

JAVAFILES=                             \
    CallReportWizard.java   		   \
    DBColumn.java           		   \
    Dataimport.java         		   \
    GroupFieldHandler.java  		   \
    IReportDocument.java    		   \
    IReportBuilderLayouter.java    		   \
    IReportDefinitionReadAccess.java    		   \
    RecordTable.java        		   \
    ReportFinalizer.java               \
    ReportImplementationHelper.java    \
    ReportLayouter.java     		   \
    ReportTextDocument.java 		   \
    ReportTextImplementation.java      \
    ReportWizard.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/report.component

$(MISC)/report.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        report.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt report.component
