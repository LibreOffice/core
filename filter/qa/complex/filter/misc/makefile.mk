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

PRJ = ..$/..$/..$/..
PRJNAME = filter
TARGET  = Filter
PACKAGE = complex$/filter$/misc

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = mysql.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES       = FinalizedMandatoryTest.java TypeDetection6FileFormat.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS  = $(PACKAGE)
JARTARGET     = $(TARGET).jar
JARCOMPRESS   = TRUE

# --- Parameters for the test --------------------------------------

# start an office if the parameter is set for the makefile
.IF "$(OFFICE)" == ""
CT_APPEXECCOMMAND =
.ELSE
CT_APPEXECCOMMAND = -AppExecutionCommand \
            "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
.ENDIF

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# replace $/ with . in package name
CT_PACKAGE  = -o $(PACKAGE:s\$/\.\)

# start the runner application
CT_APP      = org.openoffice.Runner


# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
DisplayHint : ALLTAR
.ELSE
DisplayHint : ALLDEP
.ENDIF

.INCLUDE :  target.mk

DisplayHint:
    @echo "\ntype 'dmake FinalizedMandatoryTest'"
    @echo "\ntype 'dmake TypeDetection6FileFormat'"
    @echo "! BE SHURE YOU HAVE 'TypeDetection6FileFormat.xcu' SUCCESSFUL REGISTERED IN YOU OFFICE !"

RUN: run

run: \
    DisplayHint


FinalizedMandatoryTest:
    java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_PACKAGE).FinalizedMandatoryTest
    
TypeDetection6FileFormat:
    java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_PACKAGE).TypeDetection6FileFormat

