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

PRJNAME=hyphen
TARGET=hyphen

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=hyphen-2.7.1
TARFILE_MD5=48a9f787f43a09c0a9b7b00cd1fddbbf

ADDITIONAL_FILES += makefile.mk

PATCH_FILES= \
    hyphen-2.7.1.patch \
    hyphen-2.7.1-read-charset.patch

.IF "$(GUI)"=="UNX"
CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
# still needed also in system-hyphen case as it creates the makefile
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
LCL_CONFIGURE_CFLAGS+=-m64
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

.IF "$(SYSTEM_HYPH)" == "YES" && "$(WITH_MYSPELL_DICTS)" == "YES"
BUILD_ACTION=make hyph_en_US.dic
.ELIF "$(SYSTEM_HYPH)" == "YES" && "$(WITH_MYSPELL_DICTS)" != "YES"
@all:
    echo "Nothing to do here."
.ELSE
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
OUT2INC += hyphen.h 
.ENDIF

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic

BUILD_ACTION=make

.ELSE
BUILD_ACTION=dmake
.ENDIF # "$(COM)"=="GCC"
OUT2INC += hyphen.h
.ENDIF # "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
BUILD_ACTION=dmake
OUT2INC += hyphen.h
.ENDIF # "$(GUI)"=="OS2"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

