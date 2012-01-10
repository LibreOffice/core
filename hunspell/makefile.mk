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



PRJ=.

PRJNAME=hunspell
TARGET=hunspell

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_HUNSPELL)" != "YES"

all:
    @echo "hunspell is disabled"

.ELSE

TARFILE_NAME=hunspell-1.2.9
TARFILE_MD5=68dd2e8253d9a7930e9fd50e2d7220d0
ADDITIONAL_FILES+=config.h

PATCH_FILES=\
    hunspell-wntconfig.patch \
    hunspell-solaris.patch \
    hunspell-stacksmash.patch \
    hunspell.patch

.IF "$(GUI)"=="UNX"

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) configure
CONFIGURE_FLAGS= --disable-shared --with-pic
.IF "$(COMNAME)"=="sunpro5"
CONFIGURE_FLAGS+= CFLAGS=-xc99=none
.ENDIF                  # "$(COMNAME)"=="sunpro5"

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+= CFLAGS="$(EXTRA_CFLAGS)" CXXFLAGS="$(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
CONFIGURE_FLAGS+=CPPFLAGS="$(EXTRA_CDEFS)"
.ELIF "$(OS)"=="FREEBSD" # "$(SYSBASE)"!=""
CONFIGURE_FLAGS+=CPPFLAGS="-I$(LIBINTL_PREFIX)/include" --with-libintl-prefix="$(LIBINTL_PREFIX)"
.ENDIF

BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)

OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.2.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
PATCH_FILES=\
    hunspell-mingw.patch

CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic LDFLAGS=-Wl,--enable-runtime-pseudo-reloc-v2
BUILD_ACTION=make
OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.2.a
.ELSE
BUILD_ACTION=cd src/hunspell && dmake
.ENDIF
.ENDIF # "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
BUILD_ACTION=cd src/hunspell && dmake
.ENDIF # "$(GUI)"=="OS2"

OUT2INC= \
    $(BUILD_DIR)$/src$/hunspell$/*.hxx

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
.ENDIF
