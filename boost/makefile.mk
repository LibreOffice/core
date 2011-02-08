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

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=boost
TARGET=ooo_boost

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# force patched boost for sunpro CC
# to workaround opt bug when compiling with -xO3
.IF "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
all:
    @echo "An already available installation of boost should exist on your system."        
    @echo "Therefore the version provided here does not need to be built in addition."
.ELSE			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")

# --- Files --------------------------------------------------------

TARFILE_NAME=boost_1_44_0
TARFILE_MD5=f02578f5218f217a9f20e9c30e119c6a
PATCH_FILES=$(TARFILE_NAME).patch
# See https://svn.boost.org/trac/boost/ticket/3780
PATCH_FILES+=aliasing.patch
#https://svn.boost.org/trac/boost/ticket/4713
PATCH_FILES+=boost.4713.warnings.patch
#https://svn.boost.org/trac/boost/ticket/5119
PATCH_FILES+=unordered_map-doesnt-support-cp-ctor.patch

ADDITIONAL_FILES= \
    libs/thread/src/win32/makefile.mk

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk


# --- post-build ---------------------------------------------------

# "normalize" the output structure, in that the C++ headers are
# copied to the canonic location in OUTPATH
# The allows, later on, to use the standard mechanisms to deliver those
# files, instead of delivering them out of OUTPATH/misc/build/..., which
# could cause problems

NORMALIZE_FLAG_FILE=so_normalized_$(TARGET)

$(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    -@$(MKDIRHIER) $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/*.h $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/*.hpp $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/algorithm $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/assign $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/bind $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/concept $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/config $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/date_time $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/detail $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/exception $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/function $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/functional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/io $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/integer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/iterator $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/mpl $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/numeric $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/optional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pending $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pool $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/preprocessor $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/ptr_container $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/range $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/spirit $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/smart_ptr $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/thread $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/tuple $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/type_traits $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/unordered $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/utility $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/variant $(INCCOM)$/$(PRJNAME)
    @$(TOUCH) $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

normalize: $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

.IF "$(GUI)"!="WNT"

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : normalize

.ELSE

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : boostthreadlib

boostthreadlib : $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)
    cd $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/libs/thread/src/win32 && dmake $(MFLAGS) $(CALLMACROS)

.ENDIF

.ENDIF			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
