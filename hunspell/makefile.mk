#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rene $ $Date: 2008-04-11 09:17:35 $
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

PRJNAME=hunspell
TARGET=hunspell

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=hunspell-1.1.12-2
TARFILE_ROOTDIR=hunspell-1.1.12

#ADDITIONAL_FILES += src/hunspell/makefile.mk

PATCH_FILE_NAME=hunspell-1.1.12.patch

.IF "$(GUI)"=="UNX"
#CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --without-ui --without-readline --with-pic

BUILD_ACTION=make && make check

OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.1.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
BUILD_ACTION=cd src/hunspell && dmake
.ENDIF # "$(GUI)"=="WNT"


OUT2INC= \
    $(BUILD_DIR)$/src$/hunspell$/*.hxx

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

