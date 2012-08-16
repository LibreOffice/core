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
#https://svn.boost.org/trac/boost/ticket/3780
PATCH_FILES+=boost.3780.aliasing.patch
#https://svn.boost.org/trac/boost/ticket/4127
PATCH_FILES+=boost.4127.warnings.patch
#https://svn.boost.org/trac/boost/ticket/4713
PATCH_FILES+=boost.4713.warnings.patch
#https://svn.boost.org/trac/boost/ticket/5119
PATCH_FILES+=boost.5119.unordered_map-cp-ctor.patch
#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47679
PATCH_FILES+=boost.gcc47679.patch
#https://svn.boost.org/trac/boost/ticket/6369
PATCH_FILES+=boost.6369.warnings.patch
#https://svn.boost.org/trac/boost/ticket/6397
PATCH_FILES+=boost.6397.warnings.patch
#backport from boost 1.48.0 fix for "opcode not supported on this processor"
PATCH_FILES+=boost.mipsbackport.patch

PATCH_FILES+=boost.windows.patch
PATCH_FILES+=boost.vc2012.patch

# Help static analysis tools (see SAL_UNUSED_PARAMETER in sal/types.h):
.IF "$(COM)" == "GCC"
PATCH_FILES += boost_1_44_0-unused-parameters.patch
PATCH_FILES += boost_1_44_0-logical-op-parentheses.patch
.END

# Backporting fixes for the GCC 4.7 -std=c++11 mode from Boost 1.48.0:
PATCH_FILES += boost_1_44_0-gcc4.7.patch

# Clang warnings:
PATCH_FILES += boost_1_44_0-clang-warnings.patch

# Backport http://svn.boost.org/svn/boost/trunk r76133 "Fix threading detection
# in GCC-4.7 experimental":
PATCH_FILES += boost_1_44_0-gthreads.patch

ADDITIONAL_FILES= \
    libs/thread/src/win32/makefile.mk \
	libs/date_time/src/gregorian/makefile.mk

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
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/accumulators $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/algorithm $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/archive $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/asio $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/assign $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/bimap $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/bind $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/circular_buffer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/compatibility $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/concept $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/concept_check $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/config $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/date_time $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/detail $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/dynamic_bitset $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/exception $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/filesystem $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/flyweight $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/format $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/function $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/functional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/function_types $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/fusion $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/gil $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/graph $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/integer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/interprocess $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/intrusive $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/io $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/iostreams $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/iterator $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/lambda $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/logic $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/math $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/mpi $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/mpl $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/msm $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/multi_array $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/multi_index $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/numeric $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/optional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/parameter $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pending $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/polygon $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pool $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/preprocessor $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/program_options $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/property_map $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/property_tree $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/proto $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/ptr_container $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/python $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/random $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/range $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/regex $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/serialization $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/signals $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/signals2 $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/smart_ptr $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/spirit $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/statechart $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/system $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/test $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/thread $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/tr1 $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/tuple $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/typeof $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/type_traits $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/units $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/unordered $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/utility $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/uuid $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/variant $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/wave $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/xpressive $(INCCOM)$/$(PRJNAME)
    @$(TOUCH) $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

normalize: $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

.IF "$(GUI)"!="WNT"

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : normalize boostdatetimelib

.ELSE

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : boostthreadlib boostdatetimelib

boostthreadlib : $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)
    cd $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/libs/thread/src/win32 && dmake $(MFLAGS) $(CALLMACROS)

.ENDIF

boostdatetimelib : $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)
    cd $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/libs/date_time/src/gregorian && dmake $(MFLAGS) $(CALLMACROS)

.ENDIF			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
