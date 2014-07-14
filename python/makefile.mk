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
TARFILE_MD5=1d8728eb0dfcac72a0fd99c17ec7f386
PATCH_FILES=\
    python-solaris.patch \
    python-freebsd.patch \
    python-md5.patch \
    python-ssl.patch \
    python-solver-before-std.patch \
    python-$(PYVERSION)-sysbase.patch \
    python-$(PYVERSION)-nohardlink.patch \
    python-$(PYVERSION)-pcbuild.patch

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
PATCH_FILES=python-$(PYVERSION)-mingw.patch
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
BUILD_ACTION=$(COMPATH)$/vcpackages$/vcbuild.exe -useenv pcbuild.sln "Release|Win32"
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

