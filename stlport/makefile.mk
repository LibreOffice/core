#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: hjs $ $Date: 2002-08-27 17:28:39 $
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

.IF "$(GUI)"=="WNT"
.IF "$(COMEX)"!="8"
TARFILE_NAME=STLport-4.0
PATCH_FILE_NAME=STLport-4.0.patch
.ELSE           # "$(COMEX)"!="8"
TARFILE_NAME=STLport-5.0-0409
#PATCH_FILE_NAME=STLport-4.5.3.patch
.ENDIF          # "$(COMEX)"!="8"
.ELSE
TARFILE_NAME=STLport-4.5.3
PATCH_FILE_NAME=STLport-4.5.3.patch
.ENDIF

.IF "$(USE_SHELL)"=="4nt"
TAR_EXCLUDES="*/SC5/*"
.ENDIF          # "$(GUI)"=="WNT"

ADDITIONAL_FILES=src$/gcc-3.0.mak


CONFIGURE_ACTION=none
CONFIGURE_FLAGS=

BUILD_DIR=src

.IF "$(COM)"=="MSC"
BUILD_ACTION=nmake
.IF "$(COMEX)"!="8"
BUILD_FLAGS=-f vc6.mak
.ELSE           # "$(COMEX)"!="8"
BUILD_FLAGS=-f vc7.mak
.ENDIF          # "$(COMEX)"!="8"
.ENDIF

.IF "$(COM)"=="GCC"
.IF "$(COMID)"=="gcc3"
BUILD_FLAGS=-f gcc-3.0.mak
.ELSE # "$(COMID)"=="gcc3"
# MacOS X/Darwin need a special makefile
.IF "$(OS)"=="MACOSX"
    BUILD_FLAGS=-f gcc-apple-macosx.mak
.ELSE # "$(OS)"=="MACOSX"
    BUILD_FLAGS=-f gcc.mak
.ENDIF # "$(OS)"=="MACOSX"
.ENDIF # "$(COMID)"=="gcc3"
BUILD_ACTION=make
.ENDIF

.IF "$(COM)"=="C52"
BUILD_ACTION=make
BUILD_FLAGS=-f sunpro.mak
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

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(GUI)"=="WNT"

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

ooo: $(PACKAGE_DIR)$/win32_sdk_patch 

.IF "$(USE_NEW_SDK)"!=""
$(PACKAGE_DIR)$/win32_sdk_patch :  $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.IF "$(USE_SHELL)"=="4nt"
    +win32_sdk.bat $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ELSE			# "$(USE_SHELL)"=="4nt"
    +win32_sdk.sh $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@
.ENDIF			# "$(USE_SHELL)"=="4nt"
    
$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/win32_sdk_patch
.ENDIF			# "$(USE_NEW_SDK)"!=""
.ENDIF          # "$(GUI)"=="WNT"
