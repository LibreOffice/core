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
# $Revision: 1.5 $
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

PRJ = ..$/..$/..
TARGET  = PersistentWindowTest
PRJNAME = $(TARGET)
PACKAGE = complex$/persistent_window_states

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES       = PersistentWindowTest.java DocumentHandle.java

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(TARGET)

# start the runner application
CT_APP      = org.openoffice.Runner

# --- Targets ------------------------------------------------------

$(CLASSDIR)$/$(PACKAGE)$/$(TARGET).props : ALLTAR

.INCLUDE :  target.mk

$(CLASSDIR)$/$(PACKAGE)$/$(TARGET).props : $(TARGET).props
    cp $(TARGET).props $@
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/$(TARGET).props

RUN: run

# start an office if the parameter is set for the makefile
.IF "$(OFFICE)" == ""
run:
    @echo "Execute this test with 'dmake run OFFICE=/system/path/to/office/program'."
    @echo "The office will be started by the test with a socket connection on port 8100"
.ELSE
run: $(CLASSDIR)$/$(PACKAGE)$/$(TARGET).props
    java -cp $(CLASSPATH) $(CT_APP) -AppExecutionCommand "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;" $(CT_TESTBASE) $(CT_TEST)
.ENDIF

