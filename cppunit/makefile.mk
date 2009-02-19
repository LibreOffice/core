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
# $Revision: 1.1.2.3 $
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

PRJ=.

PRJNAME=so_cppunit
TARGET=so_cppunit

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=cppunit-1.8.0

MOVED_FILES=\
 include$/cppunit$/result$/SynchronizedObject.h \
 include$/cppunit$/result$/TestListener.h \
 include$/cppunit$/result$/TestResultCollector.h \
 include$/cppunit$/result$/TestResult.h \
 include$/cppunit$/result$/TestSucessListener.h \
 include$/cppunit$/result$/TextTestResult.h \
 include$/cppunit$/portability$/config-bcb5.h \
 include$/cppunit$/portability$/config-msvc6.h \
 src$/result$/SynchronizedObject.cpp \
 src$/result$/TestResultCollector.cpp \
 src$/result$/TestResult.cpp \
 src$/result$/TestSucessListener.cpp \
 src$/result$/TextTestResult.cpp

ADDITIONAL_FILES=\
 $(MOVED_FILES) \
 include$/cppunit$/additionalfunc.hxx \
 include$/cppunit$/autoregister$/callbackfunc_fktptr.h  \
 include$/cppunit$/autoregister$/callbackstructure.h \
 include$/cppunit$/autoregister$/htestresult.h \
 include$/cppunit$/autoregister$/registerfunc.h \
 include$/cppunit$/autoregister$/registertestfunction.h \
 include$/cppunit$/autoregister$/testfunc.h \
 include$/cppunit$/checkboom.hxx \
 include$/cppunit$/cmdlinebits.hxx \
 include$/cppunit$/externcallbackfunc.hxx \
 include$/cppunit$/joblist.hxx \
 include$/cppunit$/nocopy.hxx \
 include$/cppunit$/portability$/config-auto.h \
 include$/cppunit$/result$/callbackfunc.h \
 include$/cppunit$/result$/emacsTestResult.hxx \
 include$/cppunit$/result$/log.hxx \
 include$/cppunit$/result$/optionhelper.hxx \
 include$/cppunit$/result$/outputter.hxx \
 include$/cppunit$/result$/testshlTestResult.h \
 include$/cppunit$/signaltest.h \
 include$/cppunit$/simpleheader.hxx \
 include$/cppunit$/stringhelper.hxx \
 include$/cppunit$/taghelper.hxx \
 include$/cppunit$/tagvalues.hxx \
 include$/testshl$/autoregisterhelper.hxx \
 include$/testshl$/dynamicregister.hxx \
 include$/testshl$/filehelper.hxx \
 include$/testshl$/getopt.hxx \
 include$/testshl$/log.hxx \
 include$/testshl$/tresstatewrapper.h \
 include$/testshl$/tresstatewrapper.hxx \
 include$/testshl$/versionhelper.hxx \
 include$/testshl$/winstuff.hxx \
 include$/makefile.mk \
 makefile.mk \
 src$/makefile.mk \
 src$/cppunit$/cmdlinebits.cxx \
 src$/cppunit$/joblist.cxx \
 src$/cppunit$/makefile.mk \
 src$/cppunit$/registertestfunction.cxx \
 src$/cppunit$/signaltest.cxx \
 src$/cppunit$/t_print.cxx \
 src$/cppunit$/tresregister.cxx \
 src$/cppunit$/tresstatewrapper.cxx \
 src$/result$/emacsTestResult.cxx \
 src$/result$/log.cxx \
 src$/result$/makefile.mk \
 src$/result$/optionhelper.cxx \
 src$/result$/outputter.cxx \
 src$/result$/signal.cxx \
 src$/result$/signal.hxx \
 src$/result$/testshlTestResult.cxx \
 src$/result$/treswrapper.cxx \
 src$/win32$/makefile.mk \
 src$/win32$/winstuff.cxx

PATCH_FILES=cppunit-1.8.0-r1.patch

# CONFIGURE_DIR=
# CONFIGURE_ACTION=
# 
# BUILD_DIR=
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

