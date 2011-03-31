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

PRJ = ..
TARGET  = ConnectivityComplexTests
PRJNAME = connectivity
PACKAGE = complex/connectivity

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JAVAFILES       :=  $(shell @$(FIND) complex -name "*.java")

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar

.IF "$(SYSTEM_HSQLDB)" == "YES"
EXTRAJARFILES = $(HSQLDB_JAR)
.ELSE
JARFILES += hsqldb.jar
.ENDIF

#----- make a jar from compiled files ------------------------------

JARCLASSDIRS = $(PACKAGE)
JARTARGET    = $(TARGET).jar

# --- Runner Settings ----------------------------------------------

# classpath and argument list
RUNNER_CLASSPATH = -cp "$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar"
RUNNER_ARGS = org.openoffice.Runner -TestBase java_complex

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
    @echo -----------------------------------------------------
    @echo - do a 'dmake show_targets' to show available targets
    @echo -----------------------------------------------------
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

show_targets:
    +@$(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) complexlib.ShowTargets $(foreach,i,$(JAVAFILES) $(i:s/.\$///:s/.java//))

run: $(CLASSDIR)$/$(JARTARGET)
    +$(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -sce scenarios.sce

run_%: $(CLASSDIR)$/$(JARTARGET)
    +$(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -o complex.$(PRJNAME).$(@:s/run_//)
