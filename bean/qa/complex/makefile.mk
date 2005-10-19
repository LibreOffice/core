#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-10-19 11:54:37 $
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

PRJ := ..$/..
PRJNAME := bean
TARGET := test_bean
PACKAGE = complex

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES       = OOoBeanTest.java ScreenComparer.java WriterFrame.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice


OFFICE_CLASSPATH_TMP:=$(foreach,i,$(JARFILES) $(office)$/program$/classes$/$(i)$(PATH_SEPERATOR))

OFFICE_CLASSPATH=$(OFFICE_CLASSPATH_TMP:t"")$(SOLARBINDIR)$/OOoRunner.jar$(PATH_SEPERATOR)$(CLASSDIR)

OOOBEAN_OPTIONS=-Dcom.sun.star.officebean.Options=-norestore -DOOoBean.Images=$(MISC)


.INCLUDE: target.mk

ALLTAR : RUNINSTRUCTIONS

# --- Targets ------------------------------------------------------

#The OOoBean uses the classpath to find the office installation.
#Therefore we must use the jar files from the office.
RUN:
    +java -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)
run: RUN

rund:
    +java -Xdebug -Xrunjdwp:transport=dt_socket,server=y,address=8100 -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)



RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "To run the test you have to provide the office location."
    @echo Example:
    @echo dmake run office="d:/myOffice"
    @echo .
  
