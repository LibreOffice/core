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
# $Revision: 1.13 $
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
PRJ=..$/..

PRJNAME = OOoRunner
PACKAGE = convwatch
TARGET = runner_convwatch

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar
JAVAFILES = \
 TriState.java \
 IniFile.java \
 BuildID.java \
 GfxCompare.java \
 FilenameHelper.java \
 NameHelper.java \
 HTMLOutputter.java \
 LISTOutputter.java \
 INIOutputter.java \
 PropertyName.java \
 StatusHelper.java \
 ConvWatchException.java \
 ConvWatchCancelException.java \
 OfficePrint.java \
 PRNCompare.java \
 FileHelper.java \
 OSHelper.java \
 PixelCounter.java \
 ImageHelper.java \
 BorderRemover.java \
 ConvWatch.java \
 DirectoryHelper.java \
 ConvWatchStarter.java \
 ReferenceBuilder.java \
 EnhancedComplexTestCase.java \
 MSOfficePrint.java \
 GraphicalTestArguments.java \
 StringHelper.java \
 GraphicalDifferenceCheck.java \
 DocumentConverter.java\
 DBHelper.java\
 DB.java\
 ValueNotFoundException.java \
 GlobalLogWriter.java \
 CrashLoopTest.java \
 ReportDesignerTest.java \
 DateHelper.java \
 TimeHelper.java




JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
