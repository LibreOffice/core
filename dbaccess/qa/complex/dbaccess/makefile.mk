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
# $Revision: 1.13.76.1 $
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
TARGET  = DbaComplexTests
PRJNAME = $(TARGET)
PACKAGE = complex$/dbaccess

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

.IF "$(SOLAR_JAVA)" == ""
all:
    @echo "Java not available. Build skipped"

.INCLUDE :  target.mk
.ELSE

.IF "$(BUILD_QADEVOOO)" == "YES"
#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar ConnectivityTools.jar
JAVAFILES       := $(shell @$(FIND) ./*.java)
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

RUNNER_ARGS = -cp "$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar" org.openoffice.Runner -TestBase java_complex 

RUNNER_CALL = $(AUGMENT_LIBRARY_PATH) java

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk


run: $(CLASSDIR)$/$(JARTARGET)
    +$(RUNNER_CALL) $(RUNNER_ARGS) -sce dbaccess.sce

run_%: $(CLASSDIR)$/$(JARTARGET)
    +$(RUNNER_CALL) $(RUNNER_ARGS) -o complex.dbaccess.$(@:s/run_//)

.ELSE
.INCLUDE :  target.mk
.ENDIF # "$(BUILD_QADEVOOO)" == "YES"

.ENDIF # "$(SOLAR_JAVA)" == ""
