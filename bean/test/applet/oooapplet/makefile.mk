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

PRJ := ..$/..$/..
PRJNAME := bean
TARGET := oooapplet
PACKAGE = oooapplet

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar libreoffice.jar java_uno.jar
JAVAFILES       = OOoViewer.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)


JARCLASSDIRS    = \
    oooapplet

JARTARGET       = $(TARGET).jar
JARCOMPRESS     = TRUE

#----- make a jar from compiled files ------------------------------


.INCLUDE: target.mk


ALLTAR : \
    COPY_FILES \
    RUNINSTRUCTIONS


COPY_FILES: example.html
    $(GNUCOPY) -p $< $(CLASSDIR)
# --- Targets ------------------------------------------------------


.IF "$(OS)"=="WNT"
RUN:
    firefox "$(CLASSDIR)$/example.html?$(office)"
.ELSE
TESTURL="file:///$(PWD)$/$(CLASSDIR)$/example.html?$(office)"
RUN:
    firefox ${TESTURL:s/\///}
.ENDIF

run: RUN



RUNINSTRUCTIONS :
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo .
    @echo "Add to the java runtime settings for applets in the control panel these lines:"
    @echo "-Djava.security.policy=$(PWD)$/bean.policy"
    @echo "To run the test you have to provide the office location."
    @echo Example:
    @echo dmake run office="d:\\myOffice"
    @echo .


