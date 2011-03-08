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

PRJ := ..
PRJNAME := bean
TARGET := test
PACKAGE = test

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar 
JAVAFILES       = Test.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

OFFICE_CLASSPATH_TMP:=$(foreach,i,$(JARFILES) $(office)$/program$/classes$/$(i)$(PATH_SEPERATOR))
OFFICE_CLASSPATH=$(OFFICE_CLASSPATH_TMP:t"")$(PATH_SEPERATOR)$(CLASSDIR)

OOOBEAN_OPTIONS=-Dcom.sun.star.officebean.Options=-norestore 


.INCLUDE: target.mk

ALLTAR : RUNINSTRUCTIONS

# --- Targets ------------------------------------------------------

#The OOoBean uses the classpath to find the office installation.
#Therefore we must use the jar files from the office.
RUN:
    java -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(PACKAGE).Test
run: RUN

rund:
    java -Xdebug -Xrunjdwp:transport=dt_socket,server=y,address=8100 -cp $(OFFICE_CLASSPATH) $(OOOBEAN_OPTIONS) $(PACKAGE).Test



RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "To run the test you have to provide the office location."
    @echo Example:
    @echo dmake run office="d:/myOffice"
    @echo .
  
