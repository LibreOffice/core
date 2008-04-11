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
# $Revision: 1.7 $
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
TARGET  = RecoveryTest
PRJNAME = framework
PACKAGE = complex$/framework$/recovery

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = mysql.jar mysql.jar sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES      = RecoveryTest.java RecoveryTools.java CrashThread.java TimeoutThread.java KlickButtonThread.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS  = $(PACKAGE)
JARTARGET     = $(TARGET).jar
JARCOMPRESS   = TRUE

#------ some information how to run the test -----------------------

MYTAR: ALLTAR
    @echo 
    @echo ###########################   N O T E  ######################################
    @echo 
    @echo To run the test successfully you have to extend your LD_LIBRARY_PATH
    @echo to your office program directory!
    @echo Example:
    @echo setenv LD_LIBRARY_PATH /myOffice/program:\$$LD_LIBRARY_PATH
    @echo
    @echo To run the you have to use the parameter cmd:
    @echo cmd="PATH_TO_OFFICE_BINARY -accept=socket,host=localhost,port=8100;urp;"
    @echo 
    @echo Example:
    @echo dmake run cmd="/myOffice/program/soffice -accept=socket,host=localhost,port=8100;urp;"
    @echo
    

# --- Parameters for the test --------------------------------------

# start an office if the parameter is set for the makefile
.IF "$(cmd)" == ""
CT_APPEXECCOMMAND =
.ELSE
CT_APPEXECCOMMAND = -cmd "$(cmd)"
#CT_APPEXECCOMMAND = -AppExecutionCommand \
#            "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
.ENDIF

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# replace $/ with . in package name
CT_PACKAGE  = -o $(PACKAGE:s\$/\.\)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice true
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

RUN: run
# muss noch angepasst werden: es soll auf -AppExecutionCommand und -NoOffice gepr??ft werden
#.IF "$(OFFICE)" == ""
#run:
#    @echo Exit
#.ELSE
run:
    +java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_NOOFFICE) $(CT_PACKAGE).RecoveryTest

