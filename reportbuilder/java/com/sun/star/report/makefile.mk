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



PRJ	= ..$/..$/..$/..$/..
PRJNAME = reportbuilder
TARGET=rpt_java_css
PACKAGE = com$/sun$/star$/report

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar
.IF "$(SYSTEM_APACHE_COMMONS)" == "YES"
EXTRAJARFILES = $(COMMONS_LOGGING_JAR)
.ELSE
JARFILES += commons-logging-1.1.3.jar
.ENDIF

JAVAFILES       :=	DataRow.java\
                    DataSource.java\
                    DataSourceException.java\
                    DataSourceFactory.java\
                    InputRepository.java\
                    JobDefinitionException.java\
                    JobProgressIndicator.java\
                    JobProperties.java\
                    OutputRepository.java\
                    ParameterMap.java\
                    ReportAddIn.java\
                    ReportEngine.java\
                    ReportEngineMetaData.java\
                    ReportEngineParameterNames.java\
                    ReportExecutionException.java\
                    ReportExpression.java\
                    ReportExpressionMetaData.java\
                    ReportFunction.java\
                    ReportJob.java\
                    ReportJobFactory.java\
                    ReportJobDefinition.java\
                    ImageService.java\
                    SOImageService.java\
                    StorageRepository.java\
                    SDBCReportData.java\
                    SDBCReportDataFactory.java \
                    OfficeToken.java


# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
