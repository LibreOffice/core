#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 07:51:19 $
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

PRJ=.

PRJNAME=zlib
TARGET=zlib

.IF "$(GUI)" == "UNX"
.IF "$(SYSTEM_ZLIB)" == "YES"
all:
    @echo "An already available installation of zlib should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=zlib-1.1.4

PATCH_FILE_NAME=zlib-1.1.4.patch
ADDITIONAL_FILES=makefile.mk

#relative to CONFIGURE_DIR

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    zlib.h \
    zconf.h \
    contrib$/minizip$/unzip.h



# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


