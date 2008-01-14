#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: ihi $ $Date: 2008-01-14 13:19:33 $
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
