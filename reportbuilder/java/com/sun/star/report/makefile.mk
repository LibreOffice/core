#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.8.46.2 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ	= ..$/..$/..$/..$/..
PRJNAME = reportbuilder
TARGET=rpt_java_css
PACKAGE = com$/sun$/star$/report

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar
.IF "$(SYSTEM_JFREEREPORT)" == "YES"
XCLASSPATH!:==$(XCLASSPATH)$(PATH_SEPERATOR)$(JCOMMON_JAR)$(PATH_SEPERATOR)$(COMMONS_LOGGING_JAR)
.ELSE
JARFILES += jcommon-1.0.10.jar commons-logging-1.1.1.jar
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
