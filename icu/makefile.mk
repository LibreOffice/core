#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: vg $ $Date: 2003-10-06 17:19:48 $
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=.

PRJNAME=so_icu
TARGET=so_icu

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=icu-2.2
TARFILE_ROOTDIR=icu

PATCH_FILE_NAME=icu-2.2.patch

ADDITIONAL_FILES=source$/data$/brkitr$/edit_word.txt \
        source$/data$/brkitr$/dict_word.txt \
        source$/data$/brkitr$/count_word.txt

# Currently no binary patch, but this is how it worked with ICU 2.0
#BINARY_PATCH_FILE_NAME=icu-2.0-binary_patch.tar.gz

.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
.IF "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
CC:=$(COMPATH)$/bin$/cc
CXX:=$(COMPATH)$/bin$/CC
.ENDIF          # "$(BUILD_TOOLS)$/cc"=="$(shell +-which cc)"
.ENDIF          # "$(COMNAME)"=="sunpro5"

CONFIGURE_DIR=source

CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure --enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no'

#CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
CONFIGURE_FLAGS=

# Use of
# CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure'
# CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
# doesn't work as it would result in
# sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure' --enable-layout ...
# note the position of the single quotes.

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE)
OUT2LIB= \
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST).22.0 \
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST).22 \
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST) \
    $(BUILD_DIR)$/common$/libicuuc.a \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST).22.0 \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST).22 \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST) \
    $(BUILD_DIR)$/i18n$/libicui18n.a \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST).22.0 \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST).22 \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST) \
    $(BUILD_DIR)$/layout$/libicule.a \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST).22.0 \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST).22 \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST)
.ENDIF

.IF "$(GUI)"=="WNT"
CONFIGURE_DIR=source
.IF "$(USE_SHELL)"=="4nt"
BUILD_ACTION_SEP=^
CONFIGURE_ACTION=$(BACK_PATH)..$/..$/convert.bat
.ELSE			# "$(USE_SHELL)"=="4nt"
BUILD_ACTION_SEP=;
CONFIGURE_ACTION=$(BACK_PATH)..$/..$/convert.sh
.ENDIF			# "$(USE_SHELL)"=="4nt"
BUILD_DIR=source
.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
CONFIGURE_ACTION+= $(BUILD_ACTION_SEP) wdevenv allinone$/allinone Release
BUILD_ACTION=devenv allinone$/allinone.sln /build Release /project all /useenv
.ELSE
BUILD_ACTION=msdev allinone$/allinone.dsw /useenv /MAKE "all - Win32 Release"
.ENDIF

OUT2LIB= \
    $(BUILD_DIR)$/..$/lib$/icudata.lib \
    $(BUILD_DIR)$/..$/lib$/icuin.lib \
    $(BUILD_DIR)$/..$/lib$/icuuc.lib \
    $(BUILD_DIR)$/..$/lib$/icule.lib

OUT2BIN= \
    $(BUILD_DIR)$/..$/bin$/icudt22l.dll \
    $(BUILD_DIR)$/..$/bin$/icuin22.dll \
    $(BUILD_DIR)$/..$/bin$/icuuc22.dll \
    $(BUILD_DIR)$/..$/bin$/icule22.dll
.ENDIF

# --- Targets ------------------------------------------------------

all: \
    $(MISC)$/remove_build.flag \
    ALLTAR

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(BINARY_PATCH_FILE_NAME)"!=""

$(PACKAGE_DIR)$/so_add_binary :  $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    cd $(PACKAGE_DIR) && gunzip -c $(BACK_PATH)$(BINARY_PATCH_FILE_NAME) | tar $(TAR_EXCLUDE_SWITCH) -xvf -
    +$(TOUCH) $(PACKAGE_DIR)$/so_add_binary

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_add_binary

.ENDIF

# Since you never know what will be in a patch (for example, it may already
# patch at configure level) or in the case of a binary patch, we remove the
# entire package directory if a patch is newer.
# Changes in this makefile could also make a complete build necessary if
# configure is affected.
$(MISC)$/remove_build.flag : $(BINARY_PATCH_FILE_NAME) $(PATCH_FILE_NAME) makefile.mk
    $(REMOVE_PACKAGE_COMMAND)
    +$(TOUCH) $(MISC)$/remove_build.flag

