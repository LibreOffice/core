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
# $Revision: 1.3.8.1 $
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

PRJ := ..$/..$/..
PRJNAME := bean
TARGET := oooapplet
PACKAGE = oooapplet

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar 
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


.IF "$(GUI)"=="WNT"
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
  

