#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2005-01-25 12:59:50 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
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
BUILD_ACTION=$(ENV_BUILD) gmake ; gmake install
.ELSE
BUILD_ACTION=$(ENV_BUILD) make ; make install
.ENDIF
.ELSE
# ----------------------------------
# WINDOWS
# ----------------------------------
BUILD_DIR=PCbuild
BUILD_ACTION= \
    $(foreach,i,$(PYPROJECTS) nmake -F $(i).mak CFG="$(i) - Win32 Release" OS="Windows_NT" && ) \
    echo build done
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk

# --- Targets ------------------------------------------------------

.IF "$(GUI)" != "UNX"
PYCONFIG=$(MISC)$/build$/pyconfig.h
.ENDIF

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(L10N_framework)"==""
ALLTAR : $(PYCONFIG) $(PYVERSIONFILE) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ENDIF          # "$(L10N_framework)"==""


$(MISC)$/build$/pyconfig.h : $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@

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

