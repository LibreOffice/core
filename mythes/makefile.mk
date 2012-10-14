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

PRJNAME=mythes
TARGET=mythes

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=mythes-1.2.3
TARFILE_MD5=46e92b68e31e858512b680b3b61dc4c1

ADDITIONAL_FILES += makefile.mk

PATCH_FILES=\
    mythes-1.2.0-vanilla-th-gen-idx.patch \
    mythes-1.2.0-makefile-mk.diff \
    mythes-1.2.0-android.patch \
    mythes-ssizet.patch

.IF "$(COM)"=="GCC"
.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS +:= -I$(SOLARINCDIR)$/hunspell
HUNSPELL_LIBS +:= -L$(SOLARLIBDIR) -lhunspell-1.3
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
# still needed also in system-mythes case as it creates the makefile
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
LCL_CONFIGURE_CFLAGS+=-m64
.ELIF "$(OS)"=="AIX"
LCL_CONFIGURE_CFLAG+=-D_LINUX_SOURCE_COMPAT
.ENDIF

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
LCL_CONFIGURE_CFLAGS+=$(EXTRA_CFLAGS)
CONFIGURE_FLAGS+=CXXFLAGS="$(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
CONFIGURE_FLAGS+=CPPFLAGS="$(EXTRA_CDEFS)"
.ENDIF

.IF "$(LCL_CONFIGURE_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS='$(LCL_CONFIGURE_CFLAGS)'
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no LIBS="$(STDLIBCUIMT)"
CONFIGURE_FLAGS:=$(subst,CPPRUNTIME,$(STDLIBCPP) $(CONFIGURE_FLAGS))
.ENDIF

BUILD_ACTION=make
OUT2INC += mythes.hxx
.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic \
    HUNSPELL_CFLAGS="$(HUNSPELL_CFLAGS)" \
    HUNSPELL_LIBS="$(HUNSPELL_LIBS)"

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no
.ENDIF

BUILD_ACTION=make

.ELSE
BUILD_ACTION=dmake
.ENDIF # "$(COM)"=="GCC"
OUT2INC += mythes.hxx
.ENDIF # "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(SYSTEM_HUNSPELL)" != "YES"
.EXPORT: HUNSPELL_LIBS HUNSPELL_CFLAGS
.ENDIF

