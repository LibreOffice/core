#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 15:04:26 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

# Hopefully we can move to a newer boost version soon, coming with spirit 1.8.*,
# so there won't be the need handle a separate spirit 1.6.* anymore.
# Depends on the compilers... :(

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=..

PRJNAME=ooo_boost
TARGET=ooo_spirit

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# force patched boost for sunpro CC
# to workaround opt bug when compiling with -xO3
.IF "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
all:
        @echo "An already available installation of boost should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ELSE

# --- Files --------------------------------------------------------

.IF "$(COMID)"=="gcc3" && "$(CCNUMVER)">="000400000000" 
all:
        @echo "spirit is already included in boost 1.34"
.ELSE

TARFILE_NAME=spirit-1.6.1
PATCH_FILE_NAME=$(TARFILE_NAME).patch

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

all: \
    $(MISC)$/$(TARGET)_remove_build.flag \
    ALLTAR

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

# Since you never know what will be in a patch (for example, it may already
# patch at configure level), we remove the entire package directory if a patch
# is newer.
$(MISC)$/$(TARGET)_remove_build.flag : $(PRJ)$/$(PATCH_FILE_NAME)
    $(REMOVE_PACKAGE_COMMAND)
    $(TOUCH) $(MISC)$/$(TARGET)_remove_build.flag

.ENDIF
.ENDIF
