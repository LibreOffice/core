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

PRJ=.

PRJNAME=so_berkeleydb
TARGET=so_berkeleydb

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_DB)" == "YES"
all:
    @echo "An already available installation of db should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=db-4.7.25.NC-custom
TARFILE_MD5=d70951c80dabecc2892c919ff5d07172
TARFILE_ROOTDIR=db-4.7.25.NC

ADDITIONAL_FILES= \
    makefile.mk btree$/makefile.mk clib$/makefile.mk common$/makefile.mk \
    cxx$/makefile.mk db$/makefile.mk dbm$/makefile.mk dbreg$/makefile.mk \
    env$/makefile.mk fileops$/makefile.mk hash$/makefile.mk hmac$/makefile.mk \
    hsearch$/makefile.mk lock$/makefile.mk log$/makefile.mk mp$/makefile.mk \
    mutex$/makefile.mk os$/makefile.mk os_windows$/makefile.mk \
    qam$/makefile.mk rep$/makefile.mk repmgr$/makefile.mk \
    sequence$/makefile.mk txn$/makefile.mk xa$/makefile.mk \
    db_4_7_gcc4.map

# not needed for win32. comment out when causing problems...
.IF "$(GUI)$(COM)"=="WNTGCC"
PATCH_FILES=db-4.7.25-mingw.patch
PATCH_FILES += db-4.7.25.NC-mingw32hack.patch
    # otherwise, db-4.7.25.NC/dist/configure decides for _FILE_OFFSET_BITS=64,
    # which causes /usr/i686-w64-mingw32/sys-root/mingw/include/_mingw.h to not
    # define _USE_32BIT_TIME_T, so that berkeleydb uses a 64 bit time_t while
    # the rest of LibreOffice uses a 32 bit time_t, which causes problems as
    # there is e.g. a member time_t timestamp of struct __db in db.h; see
    # <http://sourceforge.net/support/tracker.php?aid=3513251> "_mingw.h: size
    # of time_t depends on _FILE_OFFSET_BITS"
.ELSE
PATCH_FILES=\
    db-4.7.25.patch \
    db-aix.patch
.ENDIF

# clean compiler flags
CFLAGS:=
CXXFLAGS:=

# disable aliasing for all GCC platforms, at least GCC 4.x needs it if
# optimization level >= 2
.IF "$(COM)"=="GCC"
CFLAGS:=-fno-strict-aliasing $(EXTRA_CFLAGS)
CXXFLAGS:=-fno-strict-aliasing $(EXTRA_CFLAGS)
.IF "$(ENABLE_SYMBOLS)"!=""
CFLAGS+=-g
CXXFLAGS+=-g
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-z,origin -Wl,-rpath,'$$$$ORIGIN'
.EXPORT: LDFLAGS
#The current dir when linking is unxlngi6.pro/misc/build/db-4.2.52.NC/out
# the map file is in  unxlngi6.pro/misc/build/db-4.2.52.NC
LDFLAGSVERSION:= -Wl,--version-script=../db_4_7_gcc4.map
.EXPORT: LDFLAGSVERSION
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=$(ARCH_FLAGS) -R\''$$$$ORIGIN'\'
.EXPORT: LDFLAGS
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

CONFIGURE_DIR=out
#relative to CONFIGURE_DIR
CONFIGURE_ACTION= \
    ..$/dist$/configure
CONFIGURE_FLAGS=--disable-cxx --enable-dynamic --enable-compat185 CC='$(CC) $(SOLARLIB)'
.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS+= --disable-shared
.ELSE
CONFIGURE_FLAGS+= --enable-shared
.ENDIF
.IF "$(OS)"=="MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH) \
    CPPFLAGS="$(EXTRA_CDEFS)"
.ENDIF
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

# just pass ARCH_FLAGS to native build
CFLAGS+:=$(ARCH_FLAGS)
CXXFLAGS+:=$(ARCH_FLAGS)
.EXPORT : CFLAGS CXXFLAGS

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_DIR_OUT=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)

.IF "$(OS)"=="IOS"
OUT2LIB=$(BUILD_DIR)$/libdb*.a
.ELSE
OUT2LIB=$(BUILD_DIR)$/.libs$/libdb*$(DLLPOST)
.ENDIF
OUT2INC= \
    $(BUILD_DIR)$/db.h

.ENDIF			# "$(GUI)"=="UNX"

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=out
#relative to CONFIGURE_DIR
# TODO needs clean up
db_CC=$(CC) -mthreads
db_CXX=$(CXX) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
db_CC+=-shared-libgcc
db_CXX+=-shared-libgcc
.ENDIF
db_LDFLAGS=-L$(SOLARVER)/$(INPATH)/lib -L$(SOLARVER)/$(INPATH)/bin
db_LDFLAGS+=-L$(COMPATH)/lib -L$(MINGW_CLIB_DIR)
db_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
CFLAGS+=-D_GLIBCXX_DLL
db_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
db_LIBXSO_LIBS=$(db_LIBS)
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
db_LIBXSO_LIBS+=-lgcc_s
.ENDIF
CONFIGURE_ACTION=..$/dist$/configure
CONFIGURE_FLAGS=--disable-cxx --enable-dynamic --enable-shared --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --enable-mingw CC="$(db_CC)" CXX="$(db_CXX)" LN_S=ln NM="$(WRAPCMD) nm" OBJDUMP="$(WRAPCMD) objdump" JAVA="$(WRAPCMD) -env java" JAVAC="$(WRAPCMD) -env javac" CFLAGS="$(CFLAGS)" CPPFLAGS="$(INCLUDE)" LDFLAGS="$(db_LDFLAGS)" LIBS="$(db_LIBS)" LIBSO_LIBS="$(db_LIBS)" LIBJSO_LIBS="$(db_LIBS)" LIBXSO_LIBS="$(db_LIBXSO_LIBS)"

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_DIR_OUT=$(CONFIGURE_DIR)
BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/.libs$/libdb47.dll.a
OUT2BIN=$(BUILD_DIR)$/.libs$/libdb47.dll

OUT2INC= \
    $(BUILD_DIR)$/db.h
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE
BUILD_DIR=
BUILD_ACTION=dmake

BUILD_DIR_OUT=build_windows
OUT2INC= \
    $(BUILD_DIR_OUT)$/db.h
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


