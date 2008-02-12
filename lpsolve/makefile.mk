#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2008-02-12 16:04:29 $
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

PRJNAME=lpsolve
TARGET=lpsolve

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=lp_solve_5.5

.IF "$(GUI)"=="WNT"
PATCH_FILE_NAME=lp_solve_5.5-windows.patch
.ELSE
PATCH_FILE_NAME=lp_solve_5.5.patch
ADDITIONAL_FILES=lpsolve55$/ccc.solaris
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=
CONFIGURE_FLAGS=

BUILD_DIR=lpsolve55
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
BUILD_ACTION=cmd /c cgcc.bat
.ELSE
BUILD_ACTION=cmd /c cvc6.bat
OUT2LIB=$(BUILD_DIR)$/lpsolve55.lib
.ENDIF
OUT2BIN=$(BUILD_DIR)$/lpsolve55.dll
.ELSE
.IF "$(OS)"=="MACOSX"
BUILD_ACTION=sh ccc.osx
OUT2LIB=$(BUILD_DIR)$/liblpsolve55.dylib
.ELSE
.IF "$(COMNAME)"=="sunpro5"
BUILD_ACTION=sh ccc.solaris
.ELSE
BUILD_ACTION=sh ccc
.ENDIF
OUT2LIB=$(BUILD_DIR)$/liblpsolve55.so
.ENDIF
.ENDIF

OUT2INC=lp_lib.h lp_types.h lp_utils.h lp_Hash.h lp_matrix.h lp_mipbb.h lp_SOS.h

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

