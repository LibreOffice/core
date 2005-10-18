#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-10-18 08:45:54 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ = ..$/..$/..
TARGET  = TransientDocument
PRJNAME = $(TARGET)
PACKAGE = complex.tdoc

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES = CheckContentProvider.java \
            CheckTransientDocumentsContent.java \
            CheckTransientDocumentsContentProvider.java \
            CheckTransientDocumentsDocumentContent.java

#            CheckSimpleFileAccess.java \
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

# test document path
CT_TESTDOCS = -tdoc $(PWD)$/..$/test_documents

# --- Targets ------------------------------------------------------

run: ALLTAR
    +@echo .
    +@echo "The followig tests are available:"
    +@echo $(foreach,i,$(JAVAFILES) "dmake $(i:b)     ")

.INCLUDE :  target.mk

Check%:
    +java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_TESTDOCS) $(CT_APPEXECCOMMAND) $(CT_PACKAGE).$@
