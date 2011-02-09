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

# TODO: enable warnings again when external module compiles without warnings on all platforms
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=.

PRJNAME=graphite
TARGET=so_graphite

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GRAPHITE)" == "YES"
all:
        @echo "An already available installation of silgraphite should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------
.IF "$(ENABLE_GRAPHITE)"=="TRUE"
TARFILE_NAME=silgraphite-2.3.1
TARFILE_MD5=d35724900f6a4105550293686688bbb3
#graphite-updatewerror.patch -Werror passed to CFLAGS configure for
#--enable-debug, but not in configure.ac, so update configure to
#match
PATCH_FILES=\
    graphite-2.3.1.patch \
    graphite-removeobsolete.patch \
    graphite-updatewerror.patch

# convert line-endings to avoid problems when patching
CONVERTFILES=\
    engine/makefile.vc8 \
    engine/test/RegressionTest/RtTextSrc.h

CONFIGURE_DIR=engine

.IF "$(COM)"=="MSC"
.IF "$(COMEX)"=="10"
VCNUM=7
.ELSE
VCNUM=8
.ENDIF
BUILD_ACTION=nmake VERBOSE=1
.IF "$(debug)"!=""
BUILD_FLAGS= "CFG=DEBUG"
CFLAGSWITHPATH= $(CFLAGS:s!-Fd./!-Fd../../../../../!)
.ELSE
# Speed Optimization is really needed for Graphite
CFLAGSWITHPATH= $(CFLAGS) /O2
.ENDIF
### convert CFLAGS as cl.exe cannot handle OOO"s generic ones directly
### TODO: use "guw.exe" instead?
ALLCFLAGS= $(CFLAGSWITHPATH) $(CFLAGSCXX) $(CFLAGSEXCEPTIONS) $(CDEFS)
JUSTASLASH= /
CFLAGS2MSC= $(ALLCFLAGS:s/-Z/$(JUSTASLASH)Z/)
CFLAGS4MSC= $(CFLAGS2MSC:s/ -/ $(JUSTASLASH)/)
BUILD_FLAGS+= "CFLAGS4MSC=$(CFLAGS4MSC)" /F makefile.vc$(VCNUM) lib_dll
.ENDIF

.IF "$(COM)"=="GCC"

# Does linux want --disable-shared?
.IF "$(debug)"!=""
GR_CONFIGURE_FLAGS= --enable-debug=yes --disable-final --enable-static --disable-shared
.ELSE
GR_CONFIGURE_FLAGS= --enable-final=yes --enable-static --disable-shared
.ENDIF
EXTRA_GR_CXX_FLAGS=-fPIC

.IF "$(OS)"=="WNT"
PATCH_FILES+=graphite-2.3.1.patch.mingw
EXTRA_GR_CXX_FLAGS=-mthreads -nostdinc
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
EXTRA_GR_CXX_FLAGS+=-shared-libgcc
.ENDIF
EXTRA_GR_LD_FLAGS+=-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2
.ENDIF

# don't use SOLARLIB for LDFLAGS because it pulls in system graphite so build will fail
# 
CONFIGURE_ACTION=bash -c 'CXXFLAGS="$(INCLUDE) $(CFLAGSCXX) $(CFLAGSCOBJ) $(CDEFS) $(CDEFSOBJ) $(SOLARINC) $(LFS_CFLAGS) $(EXTRA_GR_CXX_FLAGS)" LDFLAGS="-L$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT) $(EXTRA_GR_LD_FLAGS)" ./configure $(GR_CONFIGURE_FLAGS)'
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
.IF "$(debug)"!=""
OUT2LIB=engine$/debug$/*.lib
.ELSE
OUT2LIB=engine$/release$/*.lib
.ENDIF
.ELSE
OUT2LIB=engine$/src$/.libs$/libgraphite*.a
.ENDIF

.IF "$(COM)"=="GCC"
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ENDIF

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libgraphite.*.dylib
.ENDIF


OUTDIR2INC= \
    engine$/include$/graphite

.IF "$(OS)"=="WNT"
OUT2INC=wrappers$/win32$/WinFont.h
.ENDIF
.ELSE
dddd:
    @echo Nothing to do
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

