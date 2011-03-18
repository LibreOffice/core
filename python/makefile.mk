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

PRJNAME=so_python
TARGET=so_python

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :      pyversion.mk

.IF "$(SYSTEM_PYTHON)" == "YES"
all:
    @echo "An already available installation of python should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF


# --- Files --------------------------------------------------------


TARFILE_NAME=Python-$(PYVERSION)
TARFILE_MD5=e81c2f0953aa60f8062c05a4673f2be0
PATCH_FILES=\
    Python-$(PYVERSION).patch \
    Python-parallel-make.patch \
    Python-ssl.patch \
    Python-2.6.1-sysbase.patch

CONFIGURE_DIR=

.IF "$(GUI)"=="UNX"
BUILD_DIR=
MYCWD=$(shell @pwd)/$(INPATH)/misc/build

# CLFLAGS get overwritten in Makefile.pre.in
.IF "$(SYSBASE)"!=""
CC+:=-I$(SYSBASE)$/usr$/include
python_LDFLAGS+=-L$(SYSBASE)/usr/lib
.IF "$(COMNAME)"=="sunpro5"
CC+:=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(CPU)"=="SOLARISU"
CC+:=$(ARCH_FLAGS)
python_LDFLAGS+=$(ARCH_FLAGS)
.ENDIF

CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) ./configure --prefix=$(MYCWD)/python-inst --enable-shared CFLAGS="$(python_CFLAGS)" LDFLAGS="$(python_LDFLAGS)"
.IF "$(OS)$(CPU)" == "SOLARISI"
CONFIGURE_ACTION += --disable-ipv6
.ENDIF
BUILD_ACTION=$(ENV_BUILD) $(GNUMAKE) -j$(EXTMAXPROCESS) && $(GNUMAKE) install && chmod -R ug+w $(MYCWD)/python-inst && chmod g+w Include
.ELSE
# ----------------------------------
# WINDOWS
# ----------------------------------
.IF "$(COM)"=="GCC"
PATCH_FILES=Python-$(PYVERSION)-mingw.patch
BUILD_DIR=
MYCWD=$(shell cygpath -m $(shell @pwd))/$(INPATH)/misc/build
python_CFLAGS=-mno-cygwin -mthreads
python_LDFLAGS=-mno-cygwin -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
python_LDFLAGS+=-shared-libgcc
.ENDIF
python_LDFLAGS+=-shared-libgcc -Wl,--enable-runtime-pseudo-reloc-v2
CONFIGURE_ACTION=./configure --prefix=$(MYCWD)/python-inst --enable-shared CC="$(CC:s/guw.exe //)" CXX="$(CXX:s/guw.exe //)" MACHDEP=MINGW32 LN="cp -p" CFLAGS="$(python_CFLAGS)" LDFLAGS="$(python_LDFLAGS)"
BUILD_ACTION=$(ENV_BUILD) make && make install
.ELSE
#PYTHONPATH:=..$/Lib
#.EXPORT : PYTHONPATH

#.IF "$(CCNUMVER)" <= "001400000000"
#EXFLAGS="/GX /YX"
#.ELSE
#.IF "$(WINDOWS_VISTA_PSDK)"!=""
#EXFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
#ADDITIONALLIBS=ws2_32.lib
#.ELSE  #"$(WINDOWS_VISTA_PSDK)"!=""
#EXFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
#.ENDIF #"$(WINDOWS_VISTA_PSDK)"!=""
#.ENDIF

BUILD_DIR=PCbuild

# Build python executable and then runs a minimal script. Running the minimal script
# ensures that certain *.pyc files are generated which would otherwise be created on
# solver during registration in insetoo_native
.IF "$(SYSBASE)" != ""
BUILD_ACTION=$(COMPATH)$/vcpackages$/vcbuild.exe -useenv pcbuild.sln "Release|Win32"
.ELSE
BUILD_ACTION=$(COMPATH)$/vcpackages$/vcbuild.exe pcbuild.sln "Release|Win32"
.ENDIF # "$(SYSBASE)" != ""
.ENDIF
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk

# --- Targets ------------------------------------------------------


.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(L10N_framework)"==""
.IF "$(GUI)" != "UNX"
.IF "$(COM)"!="GCC"
PYCONFIG:=$(MISC)$/build$/pyconfig.h
$(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)

$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PYCONFIG)

$(PYCONFIG) : $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@
.ENDIF
.ENDIF

ALLTAR : $(PYVERSIONFILE)
.ENDIF          # "$(L10N_framework)"==""


$(PYVERSIONFILE) : pyversion.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    -rm -f $@
    cat $? > $@

