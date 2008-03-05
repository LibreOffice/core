#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 17:50:37 $
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

PRJ	= ..$/..$/..$/..$/..$/..
PRJNAME = reportdesign
TARGET  = sun-report-builder
PACKAGE = com$/sun$/star$/report$/util

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

.IF "$(SYSTEM_JFREEREPORT)" == "YES"
CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(JCOMMON_JAR)$(PATH_SEPERATOR)$(LIBXML_JAR)$(PATH_SEPERATOR)$(JFREEREPORT_JAR)
.ELSE
JARFILES        = jcommon-1.0.10.jar libxml-0.9.9.jar pentaho-reporting-flow-engine-0.9.2.jar
.ENDIF

JAVAFILES       = DefaultJobProperties.java DefaultParameterMap.java DefaultReportJobDefinition.java ManifestWriter.java
# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
