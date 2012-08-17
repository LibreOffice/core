#*************************************************************************
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
#***********************************************************************/

PRJ = .
PRJNAME = cppunit
TARGET = cppunit

.INCLUDE: settings.mk

TARFILE_NAME=cppunit-1.13.0
TARFILE_MD5=0c65c839854edd43d9294d1431a2b292

PATCH_FILES = windows.patch unix.patch

.IF "$(OS)" == "ANDROID"
PATCH_FILES += android.patch
.ENDIF

.IF "$(OS)" == "WNT"
.IF "$(COM)" == "MSC"

# On Windows, CppUnit appears to support either the Unix-style configure/make
# approach with cygwin and gcc (and libtool fails miserably if gcc is replaced
# by MSVC cl), or a Visual Studio project (CppUnitLibraries.dsw).  However, the
# latter is too old to work with vcbuild from a wntmsci12 tool chain.  So, the
# easiest way appears to inject hand-crafted dmake makefiles to build the
# relevant parts.  (Another approach would be to manually use the Visual Studio
# IDE to convert CppUnitLibraries.dsw to modern format, inject the resulting
# cppunit_dll.vcproj and DllPlugInTester.vcproj and build those with vcbuild.)

CONFIGURE_ACTION = $(SED) -e s:@BACKPATH@:../$(BACK_PATH): \
    < ../$(BACK_PATH)$(PATH_IN_MODULE)/ooo-cppunit_dll.mk \
    > src/cppunit/ooo-cppunit_dll.mk && \
    $(SED) -e s:@BACKPATH@:../$(BACK_PATH): \
    < ../$(BACK_PATH)$(PATH_IN_MODULE)/ooo-DllPlugInTester.mk \
    > src/DllPlugInTester/ooo-DllPlugInTester.mk

BUILD_ACTION = cd src/cppunit && dmake -f ooo-cppunit_dll.mk debug=$(debug) verbose=$(verbose) && \
    cd ../DllPlugInTester && dmake -f ooo-DllPlugInTester.mk debug=$(debug) verbose=$(verbose)

OUTDIR2INC = include/cppunit

.INCLUDE: set_ext.mk
.INCLUDE: target.mk
.INCLUDE: tg_ext.mk

$(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE): ooo-cppunit_dll.mk ooo-DllPlugInTester.mk

.ELSE
.IF "$(COM)" == "GCC"
EXTRA_CFLAGS += -mthreads
LDFLAGS += -Wl,--enable-runtime-pseudo-reloc-v2

CONFIGURE_ACTION = ./configure
CONFIGURE_FLAGS = --prefix=$(shell cd $(PACKAGE_DIR) && \
    pwd $(PWDFLAGS))/$(TARFILE_ROOTDIR)/ooo-install \
    --disable-dependency-tracking --disable-static --disable-doxygen \
    --disable-html-docs --disable-latex-docs CC='$(CC)' CXX='$(CXX)' \
    CXXFLAGS='$(EXTRA_CFLAGS)' \
    LDFLAGS='$(LDFLAGS)' \
    LIBS='$(MY_LIBS)'

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION = $(GNUMAKE) -j$(EXTMAXPROCESS)
BUILD_FLAGS = install

OUTDIR2INC = ooo-install/include/cppunit

# WTF? A *Cygwin* DLL?
#OUT2BIN = ooo-install/bin/DllPlugInTester.exe \
#    ooo-install/bin/cygcppunit-1-13-0.dll
OUT2BIN = ooo-install/bin/DllPlugInTester.exe \
    ooo-install/bin/libcppunit-1-13-0.dll
OUT2LIB = ooo-install/lib/libcppunit.dll.a

.INCLUDE: set_ext.mk
.INCLUDE: target.mk
.INCLUDE: tg_ext.mk

.ENDIF # "$(COM)" == "GCC"
.ENDIF # "$(COM)" == "MSC"

.ELSE

# At least on Solaris with Sun CC, linking the cppunit dynamic library fails as
# TestAssert.o uses fabs but -lm is missing from the command line (cppunit's
# aclocal.m4 contains an unused AC_CHECK_LIBM, maybe using that would be a
# better fix):
.IF "$(OS)" == "SOLARIS" && "$(COM)" == "C52"
MY_LIBS = -lm
.END

.IF "$(OS)" == "ANDROID"
MY_LIBS= -lgnustl_shared -lm
.ENDIF

CONFIGURE_ACTION = ./configure

.IF "$(COM)" == "GCC"
.IF "$(OS)" == "LINUX" || "$(OS)" == "FREEBSD" || "$(OS)" == "OPENBSD" || "$(OS)" == "NETBSD" || "$(OS)" == "DRAGONFLY" || "$(OS)" == "ANDROID"
.IF "$(PRODUCT)"!="full"
# enable debug STL
EXTRA_CDEFS += -D_GLIBCXX_DEBUG
.ENDIF # !PRODUCT
.ENDIF # OS
.ENDIF # GCC

.IF "$(debug)"!=""
DEBUGFLAG=-g
.ENDIF

CONFIGURE_FLAGS = --prefix=$(shell cd $(PACKAGE_DIR) && \
    pwd $(PWDFLAGS))/$(TARFILE_ROOTDIR)/ooo-install \
	--libdir=$(shell cd $(PACKAGE_DIR) && \
	pwd $(PWDFLAGS))/$(TARFILE_ROOTDIR)/ooo-install/lib \
    --disable-dependency-tracking --disable-static --disable-doxygen \
    --disable-html-docs --disable-latex-docs CC='$(CC)' CXX='$(CXX)' \
    CXXFLAGS='$(EXTRA_CFLAGS) $(DEBUGFLAG) $(EXTRA_CDEFS)' \
    LDFLAGS='$(LDFLAGS)' \
    LIBS='$(MY_LIBS)'

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS+=--disable-shared
.ELSE
CONFIGURE_FLAGS+=--disable-static
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION = $(GNUMAKE) -j$(EXTMAXPROCESS)
BUILD_FLAGS = install

OUTDIR2INC = ooo-install/include/cppunit ooo-install/lib/pkgconfig

OUT2BIN = ooo-install/bin/DllPlugInTester
.IF "$(OS)" == "MACOSX"
OUT2LIB = ooo-install/lib/libcppunit-1.13.0.dylib
EXTRPATH = NONE
PACKAGE_DIR = \
    $(MISC)/@.__________________________________________________$(EXTRPATH)
.ELIF "$(OS)" == "AIX"
OUT2LIB = ooo-install/lib/libcppunit-1.13.a
.ELIF "$(OS)" == "OPENBSD"
OUT2LIB = ooo-install/lib/libcppunit-1.13.so.0.0
.ELIF "$(OS)" == "IOS"
OUT2LIB = ooo-install/lib/libcppunit.a
.ELIF "$(OS)" == "ANDROID"
OUT2LIB = ooo-install/lib/libcppunit-1.13.so
.ELSE
OUT2LIB = ooo-install/lib/libcppunit-1.13.so.0
.END

.INCLUDE: set_ext.mk
.INCLUDE: target.mk
.INCLUDE: tg_ext.mk

.END
