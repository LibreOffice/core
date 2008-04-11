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
# $Revision: 1.8 $
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
TARGET  = ExtensionsComplexTests
PRJNAME = extensions
PACKAGE = complex$/$(PRJNAME)

RES_TARGET = orl

.IF "$(GUI)"=="WNT"
command_seperator=&&
.ELSE
command_seperator=;
.ENDIF

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk


#----- resource files for the OfficeResourceLoader test ------------

SRS1NAME=$(RES_TARGET)_A_
SRC1FILES= \
    $(RES_TARGET)_en-US.src

RES1FILELIST=\
    $(SRS)$/$(RES_TARGET)_A_.srs

RESLIB1NAME=$(RES_TARGET)_A_
RESLIB1SRSFILES=$(RES1FILELIST)



SRS2NAME=$(RES_TARGET)_B_
SRC2FILES= \
    $(RES_TARGET)_de.src

RES2FILELIST=\
    $(SRS)$/$(RES_TARGET)_B_.srs

RESLIB2NAME=$(RES_TARGET)_B_
RESLIB2SRSFILES=$(RES2FILELIST)


#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar ConnectivityTools.jar
JAVAFILES       = $(shell @$(FIND) .$/*.java)
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

RUNNER_CLASSPATH = -cp $(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar$(PATH_SEPERATOR)$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/ConnectivityTools.jar
RUNNER_ARGS = org.openoffice.Runner -TestBase java_complex

run:copy_resources
    +java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -sce extensions_all.sce

run_%:copy_resources
    +java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -o complex.$(PRJNAME).$(@:s/run_//)


copy_resources: $(RESLIB1TARGETN) $(RESLIB2TARGETN)
    @$(foreach,i,$(RESLIB1TARGETN) $(COPY) $i $(i:s/de/invalid/:s/_A_//) $(command_seperator)) echo.
    @$(foreach,i,$(RESLIB2TARGETN) $(COPY) $i $(i:s/en-US/invalid/:s/_B_//) $(command_seperator)) echo.

