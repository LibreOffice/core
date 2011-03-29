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

PRJ=.

PRJNAME=so_stlport
TARGET=so_stlport

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(WITH_STLPORT)"!="YES"

all:
    @echo "Nothing to do"

.ELSE # "$(WITH_STLPORT)"!="YES"

# --- Files --------------------------------------------------------
.EXPORT : CC CXX
.IF "$(COMID)"=="gcc3"
    TARFILE_NAME=STLport-4.5
    TARFILE_MD5=18f577b374d60b3c760a3a3350407632
    PATCH_FILES=\
                STLport-4.5.patch \
                STLport-4.5-gcc43_warnings.patch \
                STLport-4.5-cxx0x.patch
.ELIF "$(GUI)"=="WNT"
    .IF "$(CCNUMVER)"<="001300000000"
        TARFILE_NAME=STLport-4.0
        TARFILE_MD5=c441926f3a552ed3e5b274b62e86af16
        PATCH_FILES=STLport-4.0.patch
    .ELSE
        TARFILE_NAME=STLport-4.5-0119
        TARFILE_MD5=7376930b0d3f3d77a685d94c4a3acda8
        PATCH_FILES=STLport-4.5-0119.patch
    .ENDIF
.ELSE
    TARFILE_NAME=STLport-4.0
    TARFILE_MD5=c441926f3a552ed3e5b274b62e86af16
    PATCH_FILES=STLport-4.0.patch STLport-4.0-sunstudio12u1.patch
        # To disable warnings from within STLport headers on unxsoli4 and
        # unxsols4, STLport-4.0.patch had to be extended mechanically by
        #
        #   cd unxsol.../misc/build/STLport-4.0/stlport && \
        #   find . -type f -exec sed -i \
        #     -e 's/^\([ \t]*__STL_BEGIN_NAMESPACE[ \t]*\)$/#if defined \
        #       __SUNPRO_CC\n#pragma disable_warn\n#endif\n&/' \
        #     -e 's/^\([ \t]*__STL_END_NAMESPACE[ \t]*\)$/&\n#if defined \
        #       __SUNPRO_CC\n#pragma enable_warn\n#endif/' {} \;
        #
        # (causing lots of files to become modified) and by additionally
        # changing unxsol.../misc/build/STLport-4.0/stlport/math.h,
        # unxsol.../misc/build/STLport-4.0/stlport/stl/_config.h,
        # unxsol.../misc/build/STLport-4.0/stlport/stl/_list.h, and
        # unxsol.../misc/build/STLport-4.0/stlport/stl/type_traits.h manually.
        # (Obviously due to the way the C++ compiler generates code for
        # instantiations of inline function templates from STLport headers, it
        # does not work to simply add "#pragma disable_warn" to stl/_prolog.h
        # and "#pragma enable_warn" to stl/_epilog.h, as seemingly some internal
        # STLport headers are read in by the compiler only at the end of a
        # compilation unit, outside the scope of stl/_prolog.h and
        # stl/_epilog.h.)
.ENDIF

ADDITIONAL_FILES=src$/gcc-3.0.mak src$/gcc-3.0-freebsd.mak src$/sunpro8.mak src$/sunpro11.mak src$/gcc-3.0-mingw.mak \
    src$/gcc-3.0-os2.mak src$/gcc-3.0-os2.def src$/common_macros_os2.mak


CONFIGURE_ACTION=none
CONFIGURE_FLAGS=

BUILD_DIR=src

.IF "$(COM)"=="MSC"
BUILD_ACTION=nmake
.IF "$(CCNUMVER)"<="001400000000"
BUILD_FLAGS=-f vc7.mak EXFLAGS="/EHsc"
.ELSE			# "$(CCNUMVER)"<="001400000000"
BUILD_FLAGS=-f vc7.mak EXFLAGS="/EHa /Zc:wchar_t-" CCNUMVER=$(CCNUMVER)
.ENDIF			# "$(CCNUMVER)"<="001400000000"
.ENDIF

.IF "$(COM)"=="GCC"
    .IF "$(COMID)"=="gcc3"
        # FreeBSD needs a special makefile
        .IF "$(OS)"=="FREEBSD"
            BUILD_FLAGS=-f gcc-3.0-freebsd.mak
        .ELIF "$(OS)"=="OS2"
            BUILD_FLAGS=-f gcc-3.0-os2.mak
        .ELIF "$(GUI)"=="WNT"
            BUILD_FLAGS=-f gcc-3.0-mingw.mak
        .ELSE
            BUILD_FLAGS=-f gcc-3.0.mak
        .ENDIF
    .ELSE # "$(COMID)"=="gcc3"
        .IF "$(OS)"=="FREEBSD"
            BUILD_FLAGS=-f gcc-freebsd.mak
        .ELSE
            BUILD_FLAGS=-f gcc.mak
        .ENDIF
    .ENDIF # "$(COMID)"=="gcc3"
    BUILD_ACTION=$(GNUMAKE)
    # build in parallel
    BUILD_FLAGS+= -j$(MAXPROCESS)
.ENDIF
.IF "$(HAVE_LD_HASH_STYLE)"  == "TRUE"
CXX+= -Wl,--hash-style=$(WITH_LINKER_HASH_STYLE)
.ENDIF

.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
CXX+= -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

.IF "$(COM)"=="C52"
BUILD_ACTION=make
.IF "$(CCNUMVER)">="00050008"
BUILD_FLAGS=-f sunpro11.mak
.ELIF "$(CCNUMVER)">="00050005"
BUILD_FLAGS=-f sunpro8.mak
.ELSE
BUILD_FLAGS=-f sunpro6.mak
.ENDIF # "$(CCNUMVER)">="00050008"

OUT2INC= \
    stlport$/SC5$/*.SUNWCCh
.ENDIF

OUTDIR2INC= \
    stlport

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"

OUT2LIB= \
    lib$/lib*_static.a

OUT2BIN= \
    lib$/*.dll

.ELSE

OUT2LIB= \
    lib$/*.lib

OUT2BIN= \
    lib$/*.dll \
    lib$/*.pdb

.ENDIF # "$(COM)"=="GCC"

.ELIF "$(GUI)"=="OS2"

OUT2LIB= lib$/*.lib
OUT2BIN= lib$/*.dll

.ELSE          # "$(GUI)"=="WNT"

OUT2LIB= \
    lib$/lib*

.ENDIF          # "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : 	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
.IF "$(CCNUMVER)"<="001300000000"

$(MISC)$/$(TARFILE_ROOTDIR) : avoid_win32_patches
avoid_win32_patches :
    @$(ECHONL)
    @echo ERROR! this module can't use automated patch creation
    @echo on windows.
    @$(ECHONL)
    force_dmake_to_error

$(PACKAGE_DIR)$/so_custom_patch :  $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    win32_custom.bat $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_custom_patch

.IF "$(USE_NEW_SDK)"!=""
$(PACKAGE_DIR)$/win32_sdk_patch :  $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    win32_sdk.bat $(PACKAGE_DIR) $(BACK_PATH) && $(TOUCH) $@

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/win32_sdk_patch
.ENDIF			# "$(USE_NEW_SDK)"!=""
.ENDIF			# COMVER<=001300000000
.ENDIF "$(COM)"=="GCC"
.ENDIF          # "$(GUI)"=="WNT"

.ENDIF # "$(WITH_STLPORT)"!="YES"
