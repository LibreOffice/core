#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

OFFICE_CLASSPATH_TMP:=$(foreach,i,$(JARFILES) $(office)$/program$/classes$/$(i)$(PATH_SEPARATOR))
OFFICE_CLASSPATH=$(OFFICE_CLASSPATH_TMP:t"")$(PATH_SEPARATOR)$(CLASSDIR)

OOOBEAN_OPTIONS=-Dcom.sun.star.officebean.Options=--norestore


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
  
