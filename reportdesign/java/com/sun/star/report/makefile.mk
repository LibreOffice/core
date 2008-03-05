#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 17:28:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software  you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY  without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library  if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ	= ..$/..$/..$/..$/..
PRJNAME = reportdesign
TARGET  = sun-report-builder
PACKAGE = com$/sun$/star$/report

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar
.IF "$(SYSTEM_JFREEREPORT)" == "YES"
XCLASSPATH!:==$(XCLASSPATH)$(PATH_SEPERATOR)$(JCOMMON_JAR)
.ELSE
JARFILES += jcommon-1.0.10.jar
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
                    SDBCReportDataFactory.java


# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
