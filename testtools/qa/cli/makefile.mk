#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2005-09-23 11:47:44 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#*************************************************************************

PRJ := ..$/..
PRJNAME := testtools
TARGET := test_cli
PACKAGE = clitest

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES       = CLITest.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

ALLTAR:

EXETARGET = $(BIN)$/cli_bridgetest_inprocess.exe
EXEARG_WIN= $(BIN)$/cli_bridgetest_inprocess.ini

EXEARG= $(strip $(subst,$/,/ $(EXEARG_WIN))) 

.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN:
.IF "$(GUI)"=="WNT" 
    +java -cp $(CLASSPATH) -Dcli_test=$(EXETARGET) -Dcli_test_arg=$(EXEARG) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST) 	
.ENDIF
run: RUN
