#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 17:13:47 $
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

PRJNAME=so_stlport
TARGET=so_stlport

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------
.EXPORT : CC CXX
.IF "$(COMID)"=="gcc3"
    TARFILE_NAME=STLport-4.5
    PATCH_FILE_NAME=$(MISC)$/STLport-4.5.patch
.ELSE			# "$(COMID)"=="gcc3"
    .IF "$(OS)"=="MACOSX"
        # [ed] For gcc2, we need to use STLport 4.0.  4.5 will not compile with gcc2 on OS X.
        TARFILE_NAME=STLport-4.0
        PATCH_FILE_NAME=STLport-4.0.macosx.patch
    .ELSE
        TARFILE_NAME=STLport-4.0
        PATCH_FILE_NAME=STLport-4.0.patch
    .ENDIF			# "$(OS)"=="MACOSX"
.ENDIF			# "$(COMID)"=="gcc3"

.IF "$(GUI)"=="WNT"
.IF "$(CCNUMVER)"<="001300000000"
TARFILE_NAME=STLport-4.0
PATCH_FILE_NAME=STLport-4.0.patch
.ELSE			# "$(CCNUMVER)"<="001300000000"
TARFILE_NAME=STLport-4.5-0119
PATCH_FILE_NAME=STLport-4.5-0119.patch
.ENDIF			# "$(CCNUMVER)"<="001300000000"
.ENDIF

.IF "$(USE_SHELL)"=="4nt"
TAR_EXCLUDES=*/SC5/*
.ENDIF          # "$(USE_SHELL)"=="4nt"

ADDITIONAL_FILES=src$/gcc-3.0.mak src$/gcc-3.0-macosx.mak src$/gcc-3.0-freebsd.mak


CONFIGURE_ACTION=none
CONFIGURE_FLAGS=

BUILD_DIR=src

.IF "$(COM)"=="MSC"
BUILD_ACTION=nmake
.IF "$(CCNUMVER)"<="001300000000"
BUILD_FLAGS=-f vc6.mak
.ELSE			# "$(CCNUMVER)"<="001300000000"
BUILD_FLAGS=-f vc7.mak
.ENDIF			# "$(CCNUMVER)"<="001300000000"
.ENDIF

.IF "$(COM)"=="GCC"
    .IF "$(COMID)"=="gcc3"
        # FreeBSD needs a special makefile
        .IF "$(OS)"=="FREEBSD"
            BUILD_FLAGS=-f gcc-3.0-freebsd.mak
        .ELIF "$(OS)"=="MACOSX"
            BUILD_FLAGS=-f gcc-3.0-macosx.mak
        .ELSE
            BUILD_FLAGS=-f gcc-3.0.mak
        .ENDIF
    .ELSE # "$(COMID)"=="gcc3"
        # MacOS X/Darwin need a special makefile
        .IF "$(OS)"=="MACOSX"
            BUILD_FLAGS=-f gcc-apple-macosx.mak
        .ELIF "$(OS)"=="FREEBSD"
            BUILD_FLAGS=-f gcc-freebsd.mak
        .ELSE # "$(OS)"=="MACOSX"
            BUILD_FLAGS=-f gcc.mak
        .ENDIF # "$(OS)"=="MACOSX"
    .ENDIF # "$(COMID)"=="gcc3"
    BUILD_ACTION=$(GNUMAKE)
    # build in parallel
    BUILD_FLAGS+= -j$(MAXPROCESS)
.ENDIF

.IF "$(COM)"=="C52"
BUILD_ACTION=make
BUILD_FLAGS=-f sunpro6.mak

OUT2INC= \
    stlport$/SC5$/*.SUNWCCh
.ENDIF

.IF "$(OS)"=="IRIX"
TARFILE_NAME=STLport-4.5
PATCH_FILE_NAME=$(MISC)$/STLport-4.5.patch
BUILD_ACTION=gmake
BUILD_FLAGS=-f gcc-3.0.mak
BUILD_FLAGS+= -j$(MAXPROCESS)
.ENDIF

OUTDIR2INC= \
    stlport

.IF "$(GUI)"=="WNT"

OUT2LIB= \
    lib$/*.lib

OUT2BIN= \
    lib$/*.dll \
    lib$/*.pdb

.ELSE          # "$(GUI)"=="WNT"

OUT2LIB= \
    lib$/lib*

.ENDIF          # "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.IF "$(STLPORT4)"!="NO_STLPORT4"
all :
       @echo "         An already available installation of STLport has been chosen in the configure process."
       @echo "         Therefore the version provided here does not need to be built in addition."
       +$(COPY) $(STLPORT4)$/lib$/*stlport*$(DLLPOST) $(DLLDEST)
.ENDIF

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(MISC)$/STLport-4.5.patch

$(MISC)$/STLport-4.5.patch : STLport-4.5.patch $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    +$(SED)	-e 's#GXX_INCLUDE_PATH#$(GXX_INCLUDE_PATH)#g' < STLport-4.5.patch > $(MISC)$/STLport-4.5.patch

.IF "$(GUI)"=="WNT"
.IF "$(CCNUMVER)"<="001300000000"

$(MISC)$/$(TARFILE_ROOTDIR) : avoid_win32_patches
avoid_win32_patches :
    @+$(ECHONL)
    @+echo ERROR! this module can't use automated patch creation
    @+echo on windows.
    @+$(ECHONL)
    force_dmake_to_error

$(PACKAGE_DIR)$/so_custom_patch :  $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.IF "$(USE_SHELL)"=="4nt"
    +win32_custom.bat $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ELSE			# "$(USE_SHELL)"=="4nt"
    +win32_custom.sh $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ENDIF			# "$(USE_SHELL)"=="4nt"

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_custom_patch

.IF "$(USE_NEW_SDK)"!=""
$(PACKAGE_DIR)$/win32_sdk_patch :  $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.IF "$(USE_SHELL)"=="4nt"
    +win32_sdk.bat $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ELSE			# "$(USE_SHELL)"=="4nt"
    +win32_sdk.sh $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ENDIF			# "$(USE_SHELL)"=="4nt"

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/win32_sdk_patch
.ENDIF			# "$(USE_NEW_SDK)"!=""
.ENDIF			# COMVER<=001300000000
.ENDIF          # "$(GUI)"=="WNT"
