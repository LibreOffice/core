#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-04-10 09:08:27 $
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

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=xpdf
TARGET=xpdflib

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=xpdf-3.02
PATCH_FILE_NAME=$(TARFILE_NAME).patch

CONFIGURE_DIR=
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(GUI)"=="UNX"
#CONFIGURE_ACTION=./configure
#CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions CFLAGS="-g -O0" CXXFLAGS="-g -O0"
CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
CONFIGURE_ACTION=
BUILD_ACTION= cmd.exe /c ms_make.bat
.ENDIF

CONVERTFILES=ms_make.bat
BUILD_FLAGS=

OUT2INC= \
    fofi$/*.h \
    goo$/*.h  \
    xpdf$/*.h \
    aconf.h   \
    aconf2.h

.IF "$(GUI)"=="UNX"
OUT2LIB= \
    fofi$/lib*.a \
    goo$/lib*.a \
    xpdf$/lib*.a
.ELSE
OUT2LIB= \
    fofi$/*.lib \
    goo$/*.lib \
    xpdf$/*.lib
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
