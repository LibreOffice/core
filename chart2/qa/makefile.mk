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

PRJNAME	= chart2
PRJ		= ..
TARGET	= chart_qa

PACKAGE = qa

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

MAXLINELENGTH = 100000

# - rdb ----

RDB 		= $(SOLARBINDIR)$/types.rdb
JAVADIR 	= $(OUT)$/misc$/java
JARFILES	= ridl.jar

# --- Files --------------------------------------------------------

JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/TestCaseOldAPI.class

JARFILES  = ridl.jar jurt.jar unoil.jar juh.jar OOoRunner.jar
JAVAFILES = $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

# --- Dependencies -------------------------------------------------

# Note: define dependencies after including target.mk (otherwise no targets are
# defined)

ALLTAR: runtest

# --- Rules --------------------------------------------------------

runtest: $(subst,.class,.run $(JAVACLASSFILES))

%.run: %.class
    +@java -classpath $(CLASSPATH) org.openoffice.Runner -TestBase java_complex -o $(subst,$(CLASSDIR)$/$(PACKAGE)$/,$(PACKAGE). $(subst,.class, $<))
