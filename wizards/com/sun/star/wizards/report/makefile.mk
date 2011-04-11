#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ		= ..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = report
PACKAGE = com$/sun$/star$/wizards$/report

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

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
