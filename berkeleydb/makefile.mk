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
# $Revision: 1.47 $
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

TARFILE_NAME=db-4.2.52.NC-custom
TARFILE_ROOTDIR=db-4.2.52.NC

ADDITIONAL_FILES=    \
    makefile.mk btree$/makefile.mk clib$/makefile.mk common$/makefile.mk  \
    cxx$/makefile.mk db$/makefile.mk dbm$/makefile.mk dbreg$/makefile.mk  \
    db_printlog$/makefile.mk env$/makefile.mk fileops$/makefile.mk hash$/makefile.mk  \
    hmac$/makefile.mk hsearch$/makefile.mk lock$/makefile.mk  \
    log$/makefile.mk mp$/makefile.mk mutex$/makefile.mk os$/makefile.mk  \
    os_win32$/makefile.mk qam$/makefile.mk rep$/makefile.mk txn$/makefile.mk  \
    xa$/makefile.mk libdb42.dxp db_4_2_gcc3.map



# not needed for win32. comment out when causing problems...
.IF "$(GUI)$(COM)"=="WNTGCC"
PATCH_FILE_NAME=db-4.2.52-mingw.patch
.ELSE
PATCH_FILE_NAME=db-4.2.52.patch
.ENDIF

# clean compiler flags
CFLAGS:=
CXXFLAGS:=

# disable aliasing for all GCC platforms, at least GCC 4.x needs it if
# optimization level >= 2
.IF "$(COM)"=="GCC"
CFLAGS:=-fno-strict-aliasing $(EXTRA_CFLAGS)
CXXFLAGS:=-fno-strict-aliasing $(EXTRA_CFLAGS)
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(OS)$(COM)"=="LINUXGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN' -Wl,-z,noexecstack
.EXPORT: LDFLAGS
#The current dir when linking is unxlngi6.pro/misc/build/db-4.2.52.NC/out
# the map file is in  unxlngi6.pro/misc/build/db-4.2.52.NC
LDFLAGSVERSION:= -Wl,--version-script=../db_4_2_gcc3.map
.EXPORT: LDFLAGSVERSION
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
#.IF "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
#CC:=$(COMPATH)$/bin$/cc
#CXX:=$(COMPATH)$/bin$/CC
#.ENDIF          # "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
LDFLAGS:=-R\''$$$$ORIGIN'\'
.EXPORT: LDFLAGS
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"
CONFIGURE_DIR=out
#relative to CONFIGURE_DIR
CONFIGURE_ACTION= \
    ..$/dist$/configure
CONFIGURE_FLAGS=--disable-cxx --enable-dynamic --enable-shared --enable-compat185

# just pass ARCH_FLAGS to native build
CFLAGS+:=$(ARCH_FLAGS)
CXXFLAGS+:=$(ARCH_FLAGS)
.EXPORT : CFLAGS CXXFLAGS

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_DIR_OUT=$(CONFIGURE_DIR)
.IF "$(OS)"=="IRIX"
CONFIGURE_ACTION= $(CONFIG_SHELL) ..$/dist$/configure
BUILD_ACTION=gmake 
.ELSE
BUILD_ACTION=make
.ENDIF

OUT2LIB=$(BUILD_DIR)$/.libs$/libdb*$(DLLPOST)
OUT2INC= \
    $(BUILD_DIR)$/db.h

.ENDIF			# "$(GUI)"=="UNX"

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=out
#relative to CONFIGURE_DIR
# TODO needs clean up
CFLAGS+=-nostdinc -D_MT
CONFIGURE_ACTION=..$/dist$/configure
CONFIGURE_FLAGS=--enable-cxx --enable-dynamic --enable-shared --build=i586-pc-mingw32 --host=i586-pc-mingw32 --enable-mingw LN_S=ln NM="$(WRAPCMD) nm" OBJDUMP="$(WRAPCMD) objdump" JAVA="$(WRAPCMD) -env java" JAVAC="$(WRAPCMD) -env javac" CFLAGS="$(CFLAGS)" CPPFLAGS="$(INCLUDE)" LIBS="-lmingwthrd" LIBSO_LIBS="-lmingwthrd" LIBJSO_LIBS="-lmingwthrd" LIBXSO_LIBS="-lmingwthrd $(LIBSTLPORT)"
.IF "$(USE_MINGW)"=="cygwin"
CONFIGURE_FLAGS+=LDFLAGS="-no-undefined -L$(SOLARVER)/$(INPATH)/lib -L$(SOLARVER)/$(INPATH)/bin -L$(COMPATH)/lib/mingw -L$(COMPATH)/lib/w32api -L$(COMPATH)/lib"
.ELSE
CONFIGURE_FLAGS+=LDFLAGS="-no-undefined -L$(SOLARVER)/$(INPATH)/lib -L$(SOLARVER)/$(INPATH)/bin -L$(COMPATH)/lib"
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_DIR_OUT=$(CONFIGURE_DIR)
BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/.libs$/libdb*42.a
OUT2BIN=$(BUILD_DIR)$/.libs$/libdb*42.dll

OUT2INC= \
    $(BUILD_DIR)$/db.h
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE

.IF "$(USE_SHELL)"!="4nt"
BUILD_ACTION_SEP=;
.ELSE # "$(USE_SHELL)"!="4nt"
BUILD_ACTION_SEP=^
.ENDIF # "$(USE_SHELL)"!="4nt"
BUILD_DIR=
BUILD_ACTION=dmake

BUILD_DIR_OUT=build_win32
#OUT2LIB= \
#	$(BUILD_DIR_OUT)$/Release$/libdb42.lib
#OUT2BIN=$(BUILD_DIR_OUT)$/Release$/libdb42.dll
OUT2INC= \
    $(BUILD_DIR_OUT)$/db.h
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


