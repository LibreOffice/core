#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=.

PRJNAME=so_stlport
TARGET=so_stlport

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(USE_SYSTEM_STL)"=="YES"

.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
# System STL when building with SunStudio is just a version of STLport
# which comes with the compiler
all:
    @echo "Nothing to do"
.ELSE #"$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
#
# If you choose to build without stlport, some headers will be used to bring the
# sgi extensions into the std namespace:
$(INCCOM)$/stlport$/functional \
$(INCCOM)$/stlport$/hash_map \
$(INCCOM)$/stlport$/hash_set \
$(INCCOM)$/stlport$/numeric \
$(INCCOM)$/stlport$/slist \
$(INCCOM)$/stlport$/rope \
$(INCCOM)$/stlport$/vector: systemstl$/$$(@:f)
    $(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ENDIF #"$(OS)"=="SOLARIS" && "$(COM)"!="GCC"

.ELSE # "$(USE_SYSTEM_STL)"

# --- Files --------------------------------------------------------
.EXPORT : CC CXX
.IF "$(COMID)"=="gcc3"
    TARFILE_NAME=STLport-4.5
    TARFILE_MD5=18f577b374d60b3c760a3a3350407632
    PATCH_FILES=STLport-4.5.patch STLport-4.5-gcc43_warnings.patch
.ELIF "$(GUI)"=="WNT"
    TARFILE_NAME=STLport-4.5-0119
    TARFILE_MD5=7376930b0d3f3d77a685d94c4a3acda8
    PATCH_FILES=STLport-4.5-0119.patch
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
BUILD_FLAGS=-f vc7.mak EXFLAGS="/EHa /Zc:wchar_t-" CCNUMVER=$(CCNUMVER)
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
CXX+= -Wl,--hash-style=both
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

.IF "$(STLPORT4)"!="NO_STLPORT4"
all :
       @echo "         An already available installation of STLport has been chosen in the configure process."
       @echo "         Therefore the version provided here does not need to be built in addition."
.ELIF "$(OS)"=="MACOSX"
all:
    @echo '--with-stlport=yes is not supported on Mac OS X'
    false
.ENDIF

.INCLUDE : 	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.ENDIF # "$(USE_SYSTEM_STL)"
