#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: hr $ $Date: 2005-09-26 13:00:50 $
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
    _testcapi		\
    _symtable		\
    _sre			\
    _socket			\
    _csv			\
    _bsddb			\
    pythoncore

ADDITIONAL_FILES_TMP=$(PYPROJECTS) $(PYADDITIONAL_PROJECTS)
ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES_TMP) PCbuild/$(i).mak PCbuild/$(i).dep)

CONFIGURE_DIR=

.IF "$(GUI)"=="UNX"
BUILD_DIR=
MYCWD=$(shell pwd)/$(INPATH)/misc/build
CONFIGURE_ACTION= ./configure --prefix=$(MYCWD)/python-inst --enable-shared
.IF "$(OS)$(CPU)" == "SOLARISI"
CONFIGURE_ACTION += --disable-ipv6
.ENDIF
.IF "$(COMNAME)"=="sunpro5"
.IF "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
CC:=$(COMPATH)$/bin$/cc
CXX:=$(COMPATH)$/bin$/CC
.ENDIF          # "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
.ENDIF          # "$(COMNAME)"=="sunpro5"

.IF "$(OS)" == "IRIX"
BUILD_ACTION=$(ENV_BUILD) gmake -j$(EXTMAXPROCESS) ; gmake install
.ELSE
BUILD_ACTION=$(ENV_BUILD) $(GNUMAKE) -j$(EXTMAXPROCESS) ; $(GNUMAKE) install ; chmod -R g+w $(MYCWD)/python-inst
.ENDIF
.ELSE
# ----------------------------------
# WINDOWS
# ----------------------------------
PYTHONPATH:=..$/Lib
.EXPORT : PYTHONPATH

BUILD_DIR=PCbuild
# Build python executable and then runs a minimal script. Running the minimal script
# ensures that certain *.pyc files are generated which would otherwise be created on
# solver during registration in insetoo_native
BUILD_ACTION= \
    $(foreach,i,$(PYPROJECTS) nmake /e /f $(i).mak CFG="$(i) - Win32 Release" OS="Windows_NT" && ) \
    python.exe -c "import os" && \
    echo build done
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk

# --- Targets ------------------------------------------------------


.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(L10N_framework)"==""
.IF "$(GUI)" != "UNX"
PYCONFIG:=$(MISC)$/build$/pyconfig.h
$(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)

$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PYCONFIG)

$(PYCONFIG) : $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@
.ENDIF

ALLTAR : $(PYVERSIONFILE)
.ENDIF          # "$(L10N_framework)"==""


$(PYVERSIONFILE) : pyversion.mk $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    -rm -f $@
    cat $? > $@

#--------------------
# to be moved to tg_ext.mk
$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.bz2
    @+-$(RM) $@
.IF "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := sh -c "bzip2 -cd $(BACK_PATH)download$/$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ") > $(NULLDEV)
.ELSE			# "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := bzip2 -cd $(BACK_PATH)download$/$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ) > $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@

