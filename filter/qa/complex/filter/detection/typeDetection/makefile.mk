#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
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
CT_APPEXECCOMMAND = -AppExecutionCommand "$(OFFICE)$/soffice --accept=socket,host=localhost,port=8100;urp;"
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



