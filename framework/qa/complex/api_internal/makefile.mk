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
# $Revision: 1.4 $
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
TARGET  = CheckAPI
PRJNAME = $(TARGET)
PACKAGE = complex$/api_internal

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = mysql.jar sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar mysql.jar
JAVAFILES       = CheckAPI.java
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
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL: ALLTAR $(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props
.ELSE
ALL: ALLDEP $(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props
.ENDIF

$(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props : $(JAVAFILES:b).props
    cp $(JAVAFILES:b).props $(CLASSDIR)$/$(PACKAGE)$/$(JAVAFILES:b).props
    jar uf $(CLASSDIR)$/$(JARTARGET) -C $(CLASSDIR) $(PACKAGE)$/$(JAVAFILES:b).props

.INCLUDE :  target.mk

RUN:
    +java -cp $(CLASSPATH) $(CT_APP) $(CT_APPEXECCOMMAND) $(CT_TESTBASE) $(CT_TEST)

run: RUN


