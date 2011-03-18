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

.IF "$(OOO_JUNIT_JAR)" == ""
nothing .PHONY:
    @echo -----------------------------------------------------
    @echo - JUnit not available, not building anything
    @echo -----------------------------------------------------
.ELSE   # IF "$(OOO_JUNIT_JAR)" != ""

PRJ = ../../..
PRJNAME = toolkit
TARGET = qa_complex_toolkit
PACKAGE = complex/toolkit

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = OOoRunnerLight.jar ridl.jar test.jar unoil.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

JAVAFILES = \
    $(JAVATESTFILES) \
    accessibility/_XAccessibleComponent.java \
    accessibility/_XAccessibleContext.java \
    accessibility/_XAccessibleEventBroadcaster.java \
    accessibility/_XAccessibleExtendedComponent.java \
    accessibility/_XAccessibleText.java \
    Assert.java \
    awtgrid/GridDataListener.java \
    awtgrid/TMutableGridDataModel.java \
    awtgrid/DummyColumn.java \

#----- create a jar from compiled files ----------------------------

JARTARGET       = $(TARGET).jar

#----- JUnit tests class -------------------------------------------

JAVATESTFILES = \
    GridControl.java \
    UnitConversion.java \


# disabled for now - the tests fail on at least one platform
# no issue, yet (not sure this is worth it. Don't know who to give the issue to, and don't know whether the test really makes sense)
DISABLED_JAVA_TEST_FILES=\
    AccessibleStatusBar.java\
    AccessibleStatusBarItem.java


# --- Targets ------------------------------------------------------

.INCLUDE: target.mk

ALL :   ALLTAR

# --- subsequent tests ---------------------------------------------

.IF "$(OOO_SUBSEQUENT_TESTS)" != ""

.INCLUDE: installationtest.mk

ALLTAR : javatest

    # Sample how to debug
    # JAVAIFLAGS=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

.END    # "$(OOO_SUBSEQUENT_TESTS)" == ""

.END    # ELSE "$(OOO_JUNIT_JAR)" != ""
