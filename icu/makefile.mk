#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: kz $ $Date: 2004-07-30 13:01:52 $
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

TARFILE_NAME=icu-2.6
TARFILE_ROOTDIR=icu

PATCH_FILE_NAME=icu-2.6.patch

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
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST).26.0 \
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST).26 \
    $(BUILD_DIR)$/data$/out$/libicudata$(DLLPOST) \
    $(BUILD_DIR)$/common$/libicuuc.a \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST).26.0 \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST).26 \
    $(BUILD_DIR)$/common$/libicuuc$(DLLPOST) \
    $(BUILD_DIR)$/i18n$/libicui18n.a \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST).26.0 \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST).26 \
    $(BUILD_DIR)$/i18n$/libicui18n$(DLLPOST) \
    $(BUILD_DIR)$/layout$/libicule.a \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST).26.0 \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST).26 \
    $(BUILD_DIR)$/layout$/libicule$(DLLPOST) \
    $(BUILD_DIR)$/tools$/toolutil$/libicutoolutil.a \
    $(BUILD_DIR)$/tools$/toolutil$/libicutoolutil$(DLLPOST).26.0 \
    $(BUILD_DIR)$/tools$/toolutil$/libicutoolutil$(DLLPOST).26 \
    $(BUILD_DIR)$/tools$/toolutil$/libicutoolutil$(DLLPOST)

OUT2BIN= \
    $(BUILD_DIR)$/tools$/genccode$/genccode \
    $(BUILD_DIR)$/tools$/genbrk$/genbrk

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
.IF "full_debug" == ""
# Activating the debug mechanism produces incompatible libraries, you'd have
# at least to relink all modules that are directly using ICU. Note that library
# names get a 'd' appended and you'd have to edit the solenv/inc/libs.mk
# ICU*LIB macros as well. Normally you don't want all this.
#
# Instead, use the normal already existing Release build and edit the
# corresponding *.vcproj file of the section you're interested in. Make sure
# that
# - for the VCCLCompilerTool section the following line exists:
#   DebugInformationFormat="3"
# - and for the VCLinkerTool the line
#   GenerateDebugInformation="TRUE"
# Then delete the corresponding Release output directory, and delete the target
# flag files
# $(OUTPATH)/misc/build/so_built_so_icu
# $(OUTPATH)/misc/build/so_predeliver_so_icu
# and run dmake again, after which you may copy the resulting libraries to your
# OOo/SO installation.
ICU_BUILD_VERSION=Debug
ICU_BUILD_LIBPOST=d
.ELSE
ICU_BUILD_VERSION=Release
ICU_BUILD_LIBPOST=
.ENDIF
.IF "$(COMEX)"=="8"
CONFIGURE_ACTION+= $(BUILD_ACTION_SEP) wdevenv allinone$/allinone $(ICU_BUILD_VERSION)
BUILD_ACTION=devenv allinone$/allinone.sln /build $(ICU_BUILD_VERSION) /project all /useenv
.ELIF "$(COMEX)"=="10"
CONFIGURE_ACTION+= $(BUILD_ACTION_SEP) cp ..$/..$/..$/..$/..$/makefiles.zip . $(BUILD_ACTION_SEP) unzip makefiles.zip
BUILD_ACTION=cd allinone$/all && cmd /c nmake /f all.mak CFG="all - Win32 Release" && cd ..$/..
.ELSE
BUILD_ACTION=msdev allinone$/allinone.dsw /useenv /MAKE "all - Win32 $(ICU_BUILD_VERSION)"
.ENDIF

OUT2LIB= \
    $(BUILD_DIR)$/..$/lib$/icudata.lib \
    $(BUILD_DIR)$/..$/lib$/icuin$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icuuc$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icule$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icutu$(ICU_BUILD_LIBPOST).lib

OUT2BIN= \
    $(BUILD_DIR)$/..$/bin$/icudt26l.dll \
    $(BUILD_DIR)$/..$/bin$/icuin26$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icuuc26$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icule26$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icutu26$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/genccode.exe \
    $(BUILD_DIR)$/..$/bin$/genbrk.exe

.ENDIF		# "$(GUI)"=="WNT"

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

