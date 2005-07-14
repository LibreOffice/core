#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: kz $ $Date: 2005-07-14 14:59:40 $
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

PRJNAME=so_berkeleydb
TARGET=so_berkeleydb

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_DB)" == "YES"
all:
        @echo "An already available installation of db should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=db-4.2.52.NC
ADDITIONAL_FILES=    \
    makefile.mk btree$/makefile.mk clib$/makefile.mk common$/makefile.mk  \
    cxx$/makefile.mk db$/makefile.mk dbm$/makefile.mk dbreg$/makefile.mk  \
    db_printlog$/makefile.mk env$/makefile.mk fileops$/makefile.mk hash$/makefile.mk  \
    hmac$/makefile.mk hsearch$/makefile.mk libdb_java$/makefile.mk lock$/makefile.mk  \
    log$/makefile.mk mp$/makefile.mk mutex$/makefile.mk os$/makefile.mk  \
    os_win32$/makefile.mk qam$/makefile.mk rep$/makefile.mk txn$/makefile.mk  \
    xa$/makefile.mk libdb42.dxp libdb_java42.dxp



# not needed for win32. comment out when causing problems...
PATCH_FILE_NAME=db-4.2.52.patch

.IF "$(GUI)"=="UNX"
CONFIGURE_DIR=out
#relative to CONFIGURE_DIR
CONFIGURE_ACTION= \
    setenv CFLAGS "$(ARCH_FLAGS)" && \
    setenv CXXFLAGS "$(ARCH_FLAGS)" && \
    ..$/dist$/configure
CONFIGURE_FLAGS=--enable-cxx --enable-dynamic --enable-shared
.IF "$(SOLAR_JAVA)"!=""
CONFIGURE_FLAGS+=--enable-java
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_DIR_OUT=$(CONFIGURE_DIR)
.IF "$(OS)"=="IRIX"
CONFIGURE_ACTION= $(CONFIG_SHELL) ..$/dist$/configure
BUILD_ACTION=gmake
.ELSE
BUILD_ACTION=make
.ENDIF

OUT2LIB=$(BUILD_DIR)$/.libs$/libdb*$(DLLPOST)
.IF "$(OS)"=="MACOSX"
OUT2LIB+=$(BUILD_DIR)$/.libs$/libdb_java*jnilib
.ENDIF	# "$(OS)"=="MACOSX"

.IF "$(SOLAR_JAVA)"!=""
OUT2BIN=$(BUILD_DIR)$/db.jar
OUT2CLASS=$(BUILD_DIR)$/db.jar
.ENDIF

OUT2INC= \
    $(BUILD_DIR)$/db.h

.ENDIF			# "$(GUI)"=="UNX"

.IF "$(GUI)"=="WNT"
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE

.IF "$(USE_SHELL)"!="4nt"
BUILD_ACTION_SEP=;
.ELSE # "$(USE_SHELL)"!="4nt"
BUILD_ACTION_SEP=^
.ENDIF # "$(USE_SHELL)"!="4nt"
BUILD_DIR=
BUILD_ACTION=dmake

BUILD_DIR_OUT=build_win32
#OUT2LIB= \
#	$(BUILD_DIR_OUT)$/Release$/libdb_java42.lib \
#	$(BUILD_DIR_OUT)$/Release$/libdb42.lib
#OUT2BIN=$(BUILD_DIR_OUT)$/Release$/db.jar \
#	$(BUILD_DIR_OUT)$/Release$/libdb_java42.dll \
#	$(BUILD_DIR_OUT)$/Release$/libdb42.dll
#OUT2CLASS=$(BUILD_DIR_OUT)$/Release$/db.jar
OUT2INC= \
    $(BUILD_DIR_OUT)$/db.h
.ENDIF			# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


