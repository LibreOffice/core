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

TARFILE_NAME = cppunit-1.12.1
TARFILE_MD5=bd30e9cf5523cdfc019b94f5e1d7fd19
    # from <https://sourceforge.net/projects/cppunit/files/cppunit/1.12.1/
    #  cppunit-1.12.1.tar.gz/download>

PATCH_FILES = solarisfinite.patch warnings.patch windows.patch ldflags.patch aix.patch
    # solarisfinite.patch: see <https://sourceforge.net/tracker/?func=detail&
    #  aid=2912590&group_id=11795&atid=311795>
    # warnings.patch: see <https://sourceforge.net/tracker/?func=detail&
    #  aid=2912630&group_id=11795&atid=311795>

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

.IF "$(USE_SYSTEM_STL)" != "YES"

OOO_STLPORT_CXXFLAGS = -I$(SOLARINCDIR)/stl
.IF "$(USE_STLP_DEBUG)" == "TRUE"
OOO_STLPORT_CXXFLAGS += -D_STLP_DEBUG
.END
OOO_STLPORT_CXXFLAGS += -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH)

OOO_STLPORT_LDFLAGS = -L$(SOLARLIBDIR)
OOO_STLPORT_LIBS = $(LIBSTLPORT)

.END

CONFIGURE_ACTION = ./configure
CONFIGURE_FLAGS = --prefix=$(shell cd $(PACKAGE_DIR) && \
    pwd $(PWDFLAGS))/$(TARFILE_ROOTDIR)/ooo-install \
    --disable-dependency-tracking --disable-static --disable-doxygen \
    --disable-html-docs --disable-latex-docs CC='$(CC)' CXX='$(CXX)' \
    CXXFLAGS='$(EXTRA_CFLAGS) $(OOO_STLPORT_CXXFLAGS)' \
    LDFLAGS='$(LDFLAGS) $(OOO_STLPORT_LDFLAGS)' \
    LIBS='$(OOO_STLPORT_LIBS) $(MY_LIBS)'

BUILD_ACTION = $(GNUMAKE)
BUILD_FLAGS = install

OUTDIR2INC = ooo-install/include/cppunit

OUT2BIN = ooo-install/bin/DllPlugInTester.exe \
    ooo-install/bin/cygcppunit-1-12-1.dll

.INCLUDE: set_ext.mk
.INCLUDE: target.mk
.INCLUDE: tg_ext.mk

.ENDIF # "$(COM)" == "GCC"
.ENDIF # "$(COM)" == "MSC"

.ELSE

.IF "$(USE_SYSTEM_STL)" != "YES"

OOO_STLPORT_CXXFLAGS = -I$(SOLARINCDIR)/stl
.IF "$(USE_STLP_DEBUG)" == "TRUE"
OOO_STLPORT_CXXFLAGS += -D_STLP_DEBUG
.END
.IF "$(COM)" == "GCC"
OOO_STLPORT_CXXFLAGS += -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH)
.END

OOO_STLPORT_LDFLAGS = -L$(SOLARLIBDIR)
OOO_STLPORT_LIBS = $(LIBSTLPORT)

# When "checking for C compiler default output file name" configure
# unfortunately uses "$CC $CFLAGS $CPPFLAGS $LDFLAGS conftest.c $LIBS" to build
# a C program that links against $(LIBSTLPORT); at least on one unxlngi6
# machine, this has been observed to fail with "/lib/libm.so.6: undefined
# reference to `_rtld_global_ro@GLIBC_PRIVATE'" unless -lm is also specified:
.IF "$(OS)" == "LINUX" && "$(COM)" == "GCC" && "$(CPU)" == "I"
OOO_STLPORT_LIBS += -lm
# #i112124# furthermore, STLPort seems to require libstdc++
OOO_STLPORT_LIBS += -lstdc++
.END

# And later, when "checking whether the C compiler works" configure tries to
# execute that program; however, the program would fail to locate the STLport
# library (another work-around might be to add something like --as-needed around
# $(LIBSTLPORT)):
.IF "$(OS)" == "FREEBSD" || "$(OS)" == "LINUX" || "$(OS)" == "SOLARIS" || "$(OS)" == "OPENBSD"
.IF "$(LD_LIBRARY_PATH)" == ""
LD_LIBRARY_PATH := $(SOLARLIBDIR)
    # strictly speaking, this is incorrect if the LD_LIBRARY_PATH environment
    # variable is set to the empty string
.ELSE
LD_LIBRARY_PATH := $(LD_LIBRARY_PATH):$(SOLARLIBDIR)
.END
.EXPORT: LD_LIBRARY_PATH
.END

.END

# At least on Solaris with Sun CC, linking the cppunit dynamic library fails as
# TestAssert.o uses fabs but -lm is missing from the command line (cppunit's
# aclocal.m4 contains an unused AC_CHECK_LIBM, maybe using that would be a
# better fix):
.IF "$(OS)" == "SOLARIS" && "$(COM)" == "C52"
MY_LIBS = -lm
.END

CONFIGURE_ACTION = ./configure
CONFIGURE_FLAGS = --prefix=$(shell cd $(PACKAGE_DIR) && \
    pwd $(PWDFLAGS))/$(TARFILE_ROOTDIR)/ooo-install \
    --disable-dependency-tracking --disable-static --disable-doxygen \
    --disable-html-docs --disable-latex-docs CC='$(CC)' CXX='$(CXX)' \
    CXXFLAGS='$(EXTRA_CFLAGS) $(OOO_STLPORT_CXXFLAGS)' \
    LDFLAGS='$(LDFLAGS) $(OOO_STLPORT_LDFLAGS)' \
    LIBS='$(OOO_STLPORT_LIBS) $(MY_LIBS)'

BUILD_ACTION = $(GNUMAKE)
BUILD_FLAGS = install

OUTDIR2INC = ooo-install/include/cppunit

OUT2BIN = ooo-install/bin/DllPlugInTester
.IF "$(OS)" == "MACOSX"
OUT2LIB = ooo-install/lib/libcppunit-1.12.1.dylib
EXTRPATH = NONE
.ELIF "$(OS)" == "AIX"
OUT2LIB = ooo-install/lib/libcppunit-1.12.a
.ELIF "$(OS)" == "OPENBSD"
OUT2LIB = ooo-install/lib/libcppunit-1.12.so.1.0
.ELSE
OUT2LIB = ooo-install/lib/libcppunit-1.12.so.1
.END

.INCLUDE: set_ext.mk
.INCLUDE: target.mk
.INCLUDE: tg_ext.mk

.END
