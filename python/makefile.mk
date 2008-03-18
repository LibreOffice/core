#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.37 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:46:20 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
PATCH_FILE_NAME=Python-$(PYVERSION).patch

PYPROJECTS= \
    datetime 	\
    mmap		\
    parser		\
    pyexpat		\
    python 		\
    pythonw		\
    select		\
    unicodedata	\
    w9xpopen 	\
    winreg		\
    winsound	\
    _socket		\
    _csv		\
    _sre	 	\
    _symtable	\
    _testcapi

PYADDITIONAL_PROJECTS = \
    zlib 			\
    make_versioninfo	\
    bz2			\
    _tkinter		\
    _bsddb			\
    pythoncore

ADDITIONAL_FILES_TMP=$(PYPROJECTS) $(PYADDITIONAL_PROJECTS)
ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES_TMP) PCbuild/$(i).mak PCbuild/$(i).dep)

CONFIGURE_DIR=

ADDITIONAL_FILES+=Lib/plat-cygwin Python/fileblocks.c

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

.IF "$(OS)$(COM)"=="LINUXGCC"
python_LDFLAGS+=-Wl,-z,noexecstack
.ENDIF

# SunStudio on Solaris 10 and above needs the -xc99=all flag already 
# during the configuration tests, otherwise the HAVE_LIMITS_H check will
# be wrong resulting in a build breaker.
.IF "$(SYSBASE)"==""
.IF "$(COMNAME)"=="sunpro5"
CC+=-xc99=all
.ENDIF          # "$(COMNAME)"=="sunpro5"
.ENDIF


CONFIGURE_ACTION=./configure --prefix=$(MYCWD)/python-inst --enable-shared CFLAGS="$(python_CFLAGS)" LDFLAGS="$(python_LDFLAGS)"
.IF "$(OS)$(CPU)" == "SOLARISI"
CONFIGURE_ACTION += --disable-ipv6
.ENDIF
.IF "$(OS)" == "IRIX"
BUILD_ACTION=$(ENV_BUILD) gmake -j$(EXTMAXPROCESS) ; gmake install
.ELSE
BUILD_ACTION=$(ENV_BUILD) $(GNUMAKE) -j$(EXTMAXPROCESS) ; $(GNUMAKE) install ; chmod -R ug+w $(MYCWD)/python-inst
.ENDIF
.ELSE
# ----------------------------------
# WINDOWS
# ----------------------------------
.IF "$(COM)"=="GCC"
BUILD_DIR=
MYCWD=$(shell cygpath -m $(shell @pwd))/$(INPATH)/misc/build
CC:=$(CC:s/guw.exe //)
CXX:=$(CXX:s/guw.exe //)
LDFLAGS:=-mno-cygwin
.EXPORT : LDFLAGS
CONFIGURE_ACTION= ./configure --prefix=$(MYCWD)/python-inst --enable-shared LN="cp -p" LDFLAGS=-mno-cygwin
BUILD_ACTION=$(ENV_BUILD) make ; make install
.ELSE
PYTHONPATH:=..$/Lib
.EXPORT : PYTHONPATH

.IF "$(CCNUMVER)" <= "001400000000"
EXFLAGS="/GX /YX"
.ELSE
.IF "$(WINDOWS_VISTA_PSDK)"!=""
EXFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
ADDITIONALLIBS=ws2_32.lib
.ELSE  #"$(WINDOWS_VISTA_PSDK)"!=""
EXFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
.ENDIF #"$(WINDOWS_VISTA_PSDK)"!=""
.ENDIF

BUILD_DIR=PCbuild

# Build python executable and then runs a minimal script. Running the minimal script
# ensures that certain *.pyc files are generated which would otherwise be created on
# solver during registration in insetoo_native
BUILD_ACTION= \
    $(foreach,i,$(PYPROJECTS) nmake /f $(i).mak CFG="$(i) - Win32 Release" EXFLAGS=$(EXFLAGS) ADDITIONALLIBS=$(ADDITIONALLIBS) && ) \
    python.exe -c "import os" && \
    echo build done
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

