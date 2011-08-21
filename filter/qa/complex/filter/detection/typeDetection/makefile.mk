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
PRJ = ..$/..$/..$/..$/..
PRJNAME = filter
TARGET  = TypeDetection
PACKAGE = complex$/filter$/detection$/typeDetection

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES       = TypeDetection.java Helper.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Parameters for the test --------------------------------------

# start an office if the parameter is set for the makefile
.IF "$(OFFICE)" == ""
CT_APPEXECCOMMAND =
.ELSE
CT_APPEXECCOMMAND = -AppExecutionCommand "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
.ENDIF

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).TypeDetection

# start the runner application
CT_APP      = org.openoffice.Runner

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
        CHMOD $(CLASSDIR)$/$(PACKAGE)$/TypeDetection.props \
        $(CLASSDIR)$/$(PACKAGE)$/preselectedFilter.csv \
        $(CLASSDIR)$/$(PACKAGE)$/preselectedType.csv \
        $(CLASSDIR)$/$(PACKAGE)$/serviceName.csv \
        $(CLASSDIR)$/$(PACKAGE)$/files.csv : ALLTAR
.ELSE
        CHMOD $(CLASSDIR)$/$(PACKAGE)$/TypeDetection.props \
        $(CLASSDIR)$/$(PACKAGE)$/preselectedFilter.csv \
        $(CLASSDIR)$/$(PACKAGE)$/preselectedType.csv \
        $(CLASSDIR)$/$(PACKAGE)$/serviceName.csv \
        $(CLASSDIR)$/$(PACKAGE)$/files.csv : ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(CLASSDIR)$/$(PACKAGE)$/preselectedFilter.csv : preselectedFilter.csv
    cp preselectedFilter.csv $(CLASSDIR)$/$(PACKAGE)$/preselectedFilter.csv
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/preselectedFilter.csv

$(CLASSDIR)$/$(PACKAGE)$/preselectedType.csv : preselectedType.csv
    cp preselectedType.csv $(CLASSDIR)$/$(PACKAGE)$/preselectedType.csv
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/preselectedType.csv

$(CLASSDIR)$/$(PACKAGE)$/serviceName.csv : serviceName.csv
    cp serviceName.csv $(CLASSDIR)$/$(PACKAGE)$/serviceName.csv
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/serviceName.csv

$(CLASSDIR)$/$(PACKAGE)$/files.csv : files.csv
    cp files.csv $(CLASSDIR)$/$(PACKAGE)$/files.csv  
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/files.csv

$(CLASSDIR)$/$(PACKAGE)$/TypeDetection.props : TypeDetection.props
    cp TypeDetection.props $(CLASSDIR)$/$(PACKAGE)$/TypeDetection.props
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/TypeDetection.props

# --- chmod --------------------------------------------------------

CHMOD :
    chmod 444 $(CLASSDIR)$/$(PACKAGE)$/*.csv 
    chmod 666 $(CLASSDIR)$/$(PACKAGE)$/*.props 

RUN: run

run:
    java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_TEST)



