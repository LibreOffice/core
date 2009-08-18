#*************************************************************************
#
#   $RCSfile: graphite-makefile-mk.diff,v $
#
#   last change: $Author: rodo $ $Date: 2006/01/03 17:58:01 $
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

# TODO: enable warnings again when external module compiles without warnings on all platforms
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=.

PRJNAME=graphite
TARGET=so_graphite

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------
.IF "$(OS)"=="WNT" ||  "$(OS)"=="LINUX" 
TARFILE_NAME=silgraphite-2.3.1
PATCH_FILES=graphite-2.3.1.patch
.ELSE
dummy:
    echo Nothing to do for non Linux / Windows here
.ENDIF

# convert line-endings to avoid problems when patching
CONVERTFILES=\
    engine/makefile.vc8

#.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
#CONFIGURE_DIR=win32
#.ELSE
#CONFIGURE_DIR=engine
#.ENDIF

CONFIGURE_DIR=engine

.IF "$(COM)"=="MSC"
.IF "$(COMEX)"=="10"
VCNUM=7
.ELSE
VCNUM=8
.ENDIF
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE
BUILD_ACTION=nmake
.IF "$(debug)"=="true"
BUILD_FLAGS= "CFG=DEBUG" "MLIB=MD" /F makefile.vc$(VCNUM) dll
.ELSE
BUILD_FLAGS="MLIB=MD" /F makefile.vc$(VCNUM) dll
.ENDIF
.ENDIF

.IF "$(COM)"=="GCC"

# Does linux want --disable-shared?
.IF "$(debug)"=="true"
GR_CONFIGURE_FLAGS= --enable-debug=yes --disable-final --enable-static --disable-shared
.ELSE
GR_CONFIGURE_FLAGS= --enable-final=yes --enable-static --disable-shared
.ENDIF
EXTRA_GR_CXX_FLAGS=-fPIC

.IF "$(USE_SYSTEM_STL)"!="YES"
EXTRA_GR_LD_FLAGS=$(LIBSTLPORT) -lm
GR_LIB_PATH=LD_LIBRARY_PATH=$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT)
.ELSE
GR_LIB_PATH=
.ENDIF

# don't use SOLARLIB for LDFLAGS because it pulls in system graphite so build will fail
# 
CONFIGURE_ACTION=bash -c 'CXXFLAGS="$(INCLUDE) $(CFLAGSCXX) $(CFLAGSCOBJ) $(CDEFS) $(CDEFSOBJ) $(SOLARINC) $(LFS_CFLAGS) $(EXTRA_GR_CXX_FLAGS)" $(GR_LIB_PATH) LDFLAGS="-L$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT) $(EXTRA_GR_LD_FLAGS)" ./configure $(GR_CONFIGURE_FLAGS)'
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
#OUT2LIB=win32$/bin.msvc$/*.lib
.IF "$(debug)"=="true"
OUT2LIB=engine$/debug$/*.lib
.ELSE
OUT2LIB=engine$/release$/*.lib
.ENDIF
.ELSE
OUT2LIB=engine$/src$/.libs$/libgraphite*.a
.ENDIF

.IF "$(COM)"=="GCC"
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ENDIF

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libgraphite.*.dylib
.ELSE
.IF "$(OS)"=="WNT"
#OUT2LIB+=engine$/src$/.libs$/libgraphite*.dll
.IF "$(debug)"=="true"
OUT2BIN= \
    engine$/debug$/*.dll \
    engine$/debug$/*.pdb
.ELSE
OUT2BIN= \
    engine$/release$/*.dll
#    engine$/release$/*.pdb
.ENDIF
.ELSE
#OUT2LIB+=engine$/src$/.libs$/libgraphite.so.*.*.*
.ENDIF
.ENDIF


OUTDIR2INC= \
    engine$/include$/graphite

.IF "$(OS)"=="WNT"
OUT2INC=wrappers$/win32$/WinFont.h
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

