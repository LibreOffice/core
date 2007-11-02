#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 14:13:23 $
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

PRJNAME=libtextcat
TARGET=libtextcat

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=libtextcat-2.2
TARFILE_ROOTDIR=libtextcat-2.2

PATCH_FILE_NAME=libtextcat-2.2.patch


ADDITIONAL_FILES= \
                src$/utf8misc.h \
                src$/utf8misc.c \
                src$/win32_config.h \
                src$/makefile.mk \
                src$/libtextcat.map

.IF "$(GUI)"=="UNX"
#CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS=

BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/src$/.libs$/libtextcat*$(DLLPOST)

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
BUILD_ACTION=cd src && dmake $(MAKEMACROS)

.ENDIF # "$(GUI)"=="WNT" || "$(GUI)"=="OS2"


OUT2INC= \
    $(BUILD_DIR)$/src$/config.h \
    $(BUILD_DIR)$/src$/common.h \
    $(BUILD_DIR)$/src$/fingerprint.h \
    $(BUILD_DIR)$/src$/textcat.h \
    $(BUILD_DIR)$/src$/wg_mempool.h


# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

