#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

.ELIF "$(ENABLE_GRAPHITE)" != "TRUE"

all:
        @echo "Support for silgraphite has been disabled.  Nothing to do."

.ELSE

# --- Files --------------------------------------------------------

TARFILE_NAME=silgraphite-2.3.1
TARFILE_MD5=d35724900f6a4105550293686688bbb3
PATCH_FILES=graphite-2.3.1.patch graphite-2.3.1_debug.patch graphite-2.3.1_stlportemu.patch graphite-2.3.1-CVE-2016-1521.patch

# convert line-endings to avoid problems when patching
CONVERTFILES=\
    engine/makefile.vc8 \
    engine/test/RegressionTest/RtTextSrc.h

#.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
#CONFIGURE_DIR=win32
#.ELSE
#CONFIGURE_DIR=engine
#.ENDIF

CONFIGURE_DIR=engine

.IF "$(COM)"=="MSC"
.IF "$(COMEX)"=="10"
VCNUM=7
.ELSE
VCNUM=8
.ENDIF
.IF "$(USE_SYSTEM_STL)"!="YES"
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE
.ENDIF
BUILD_ACTION=nmake VERBOSE=1
.IF "x$(debug)"!="x"
BUILD_FLAGS= "CFG=DEBUG"
CFLAGSWITHPATH= $(CFLAGS:s!-Fd.!-Fd../../../../../!)
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

.IF "$(COM)"=="GCC" || "$(COM)"=="CLANG"

# Does linux want --disable-shared?
.IF "x$(debug)"!="x"
GR_CONFIGURE_FLAGS= --enable-debug=yes --disable-final --enable-static --disable-shared
.ELSE
GR_CONFIGURE_FLAGS= --enable-final=yes --enable-static --disable-shared
.ENDIF
EXTRA_GR_CXX_FLAGS=-fPIC

.IF "$(USE_SYSTEM_STL)"!="YES"
# #i112124# STLPort seems to require libstdc++
EXTRA_GR_LD_FLAGS=$(LIBSTLPORT) -lm -lstdc++
GR_LIB_PATH=LD_LIBRARY_PATH=$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT)
.ELSE
GR_LIB_PATH=
.ENDIF

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
CONFIGURE_ACTION=bash -c 'CXXFLAGS="$(INCLUDE) $(CFLAGSCXX) $(CFLAGSCOBJ) $(CDEFS) $(CDEFSOBJ) $(SOLARINC) $(LFS_CFLAGS) $(EXTRA_GR_CXX_FLAGS)" $(GR_LIB_PATH) LDFLAGS="-L$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT) $(EXTRA_GR_LD_FLAGS)" ./configure $(GR_CONFIGURE_FLAGS)'
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
#OUT2LIB=win32$/bin.msvc$/*.lib
.IF "x$(debug)"!="x"
OUT2LIB=engine$/debug$/*.lib
.ELSE
OUT2LIB=engine$/release$/*.lib
.ENDIF
.ELSE
OUT2LIB=engine$/src$/.libs$/libgraphite*.a
.ENDIF

.IF "$(COM)"=="GCC" || "$(COM)"=="CLANG"
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ENDIF

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libgraphite.*.dylib
.ELSE
.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
#OUT2LIB+=engine$/src$/.libs$/libgraphite*.dll
.IF "x$(debug)"!="x"
OUT2BIN= \
#    engine$/debug$/*.dll \
    engine$/debug$/*.pdb
.ELSE
OUT2BIN=
#    engine$/release$/*.dll
#    engine$/release$/*.pdb
.ENDIF
.ELSE
#OUT2LIB+=engine$/src$/.libs$/libgraphite.so.*.*.*
.ENDIF
.ENDIF


OUTDIR2INC= \
    engine$/include$/graphite

.IF "$(OS)"=="WNT"
OUT2INC=wrappers$/win32$/WinFont.h
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.ENDIF
