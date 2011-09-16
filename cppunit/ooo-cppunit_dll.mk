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
#***********************************************************************/

PRJ = @BACKPATH@../..
PRJNAME = cppunit
TARGET = cppunit_dll

ENABLE_EXCEPTIONS = TRUE
EXTERNAL_WARNINGS_NOT_ERRORS = TRUE
nodep = TRUE

.INCLUDE: settings.mk

CDEFS += -DCPPUNIT_BUILD_DLL
CFLAGSCXX += -I../../include

SLOFILES = $(SHL1OBJS)

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = \
    $(SLO)/AdditionalMessage.obj \
    $(SLO)/Asserter.obj \
    $(SLO)/BeosDynamicLibraryManager.obj \
    $(SLO)/BriefTestProgressListener.obj \
    $(SLO)/CompilerOutputter.obj \
    $(SLO)/DefaultProtector.obj \
    $(SLO)/DllMain.obj \
    $(SLO)/DynamicLibraryManager.obj \
    $(SLO)/DynamicLibraryManagerException.obj \
    $(SLO)/Exception.obj \
    $(SLO)/Message.obj \
    $(SLO)/PlugInManager.obj \
    $(SLO)/PlugInParameters.obj \
    $(SLO)/Protector.obj \
    $(SLO)/ProtectorChain.obj \
    $(SLO)/RepeatedTest.obj \
    $(SLO)/SourceLine.obj \
    $(SLO)/StringTools.obj \
    $(SLO)/SynchronizedObject.obj \
    $(SLO)/Test.obj \
    $(SLO)/TestAssert.obj \
    $(SLO)/TestCase.obj \
    $(SLO)/TestCaseDecorator.obj \
    $(SLO)/TestComposite.obj \
    $(SLO)/TestDecorator.obj \
    $(SLO)/TestFactoryRegistry.obj \
    $(SLO)/TestFailure.obj \
    $(SLO)/TestLeaf.obj \
    $(SLO)/TestNamer.obj \
    $(SLO)/TestPath.obj \
    $(SLO)/TestPlugInDefaultImpl.obj \
    $(SLO)/TestResult.obj \
    $(SLO)/TestResultCollector.obj \
    $(SLO)/TestRunner.obj \
    $(SLO)/TestSetUp.obj \
    $(SLO)/TestSuccessListener.obj \
    $(SLO)/TestSuite.obj \
    $(SLO)/TestSuiteBuilderContext.obj \
    $(SLO)/TextOutputter.obj \
    $(SLO)/TextTestProgressListener.obj \
    $(SLO)/TextTestResult.obj \
    $(SLO)/TextTestRunner.obj \
    $(SLO)/TypeInfoHelper.obj \
    $(SLO)/UnixDynamicLibraryManager.obj \
    $(SLO)/Win32DynamicLibraryManager.obj \
    $(SLO)/XmlDocument.obj \
    $(SLO)/XmlElement.obj \
    $(SLO)/XmlOutputter.obj \
    $(SLO)/XmlOutputterHook.obj
SHL1RPATH = NONE
SHL1TARGET = cppunit_dll
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
