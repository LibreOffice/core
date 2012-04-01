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

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=vigra
TARGET=vigra

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_VIGRA)" == "YES"
all:
    @echo "An already available installation of vigra should exist on your system."        
    @echo "Therefore the version provided here does not need to be built in addition."
.ELSE

# --- Files --------------------------------------------------------

TARFILE_NAME=vigra1.4.0
TARFILE_MD5=ea91f2fb4212a21d708aced277e6e85a
PATCH_FILES=$(TARFILE_NAME).patch

# Help static analysis tools (see SAL_UNUSED_PARAMETER in sal/types.h):
.IF "$(COM)" == "GCC"
PATCH_FILES += vigra1.4.0-unused-parameters.patch
.END

# At least GCC 4.7 treats more correctly now the type of enumerators prior to
# the closing brace of the enum-specifier (see [dcl.enum] in the C++ 2003
# Standard), leading to "comparison between <enum1> and <enum2> [-Werror=enum-
# compare]" and "enumeral mismatch in conditional expression [-Werror]"
# warnings (included upstream as <https://github.com/ukoethe/vigra/commit/
# e0dcd31c76bb13c98920e21544b309ca47bb3c5c> "added explicit cast (gcc 4.7
# compatibility patch by Stephan Bergmann)"):
PATCH_FILES += vigra1.4.0-enumwarn.patch

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF
