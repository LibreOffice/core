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

PRJ=..

PRJNAME=cairo
TARGET=so_pixman
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_CAIRO)" == "YES"
all:
    @echo "Not building pixman."

.ENDIF

# --- Files --------------------------------------------------------

PIXMANVERSION=0.24.4

TARFILE_NAME=pixman-$(PIXMANVERSION)
TARFILE_MD5=c63f411b3ad147db2bcce1bf262a0e02

ADDITIONAL_FILES=Makefile.win32.common

PATCH_FILES=..$/$(TARFILE_NAME).patch

.IF "$(OS)"=="ANDROID"
PATCH_FILES+=..$/$(TARFILE_NAME).android.patch
.ENDIF

# Note: we are building static pixman library to avoid linking problems.
# However, for Unix dynamic library must be used (especially due to 64bit issues)

.IF "$(OS)"=="WNT"
# --------- Windows -------------------------------------------------
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 CFLAGS="$(pixman_CFLAGS) -mthreads" LDFLAGS="$(pixman_LDFLAGS) -no-undefined -L$(ILIB:s/;/ -L/)" OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE   # WNT, not GCC
BUILD_DIR=pixman
BUILD_ACTION=$(GNUMAKE) -f Makefile.win32 MMX=on SSE2=on CFG=release
.ENDIF

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no CPPFLAGS="$(EXTRA_CDEFS)"
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS="$(EXTRA_CFLAGS) $(EXTRA_CDEFS)"
.ENDIF # "$(EXTRA_CDEFS)"!=""
.ENDIF # "$(SYSBASE)"!=""
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-z,origin -Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-z,noexecstack
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"

.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(SYSBASE)"!=""
pixman_CFLAGS+=-I$(SYSBASE)$/usr$/include -I$(SOLARINCDIR)$/external $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -L$(SOLARLIBDIR) -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.EXPORT: LDFLAGS

.IF "$(COMNAME)"=="sunpro5"
pixman_CFLAGS+=-xc99=none
.ENDIF

.IF "$(CPUNAME)"=="INTEL"
pixman_CFLAGS+=-march=i486
.ENDIF

pixman_CFLAGS+=-fPIC

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF

CONFIGURE_FLAGS+=CFLAGS="$(pixman_CFLAGS)"

.IF "$(OS)"=="ANDROID"

# The pixman-cpu.c code wants to read /proc/<pid>/auxv, but 
# the Android headers don't define Elf32_auxv_t.

# Maybe we should instead just patch the arm_has_* booleans in
# pixman-cpu.c to be hardcoded as TRUE and patch out the run-time
# check?

CONFIGURE_FLAGS+=--disable-arm-simd --disable-arm-neon --disable-arm-iwmmxt

.ELIF "$(OS)" == "MACOSX"

CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)

.END

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF



# -------- All platforms --------------------------------------------

OUT2INC=pixman$/pixman-version.h  \
    pixman$/pixman.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=pixman$/.libs$/libpixman-1.a
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=pixman$/.libs$/*.a
.ELSE
OUT2LIB+=pixman$/release$/*.lib
.ENDIF
.ELIF "$(OS)"=="IOS"
OUT2LIB+=pixman$/.libs$/libpixman-1.a
.ELSE
OUT2LIB+=pixman$/.libs$/libpixman-1.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

