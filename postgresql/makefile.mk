#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
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

PRJ=.

PRJNAME=postgresql
TARGET=so_postgresql

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=postgresql-9.1.1
TARFILE_MD5=061a9f17323117c9358ed60f33ecff78
PATCH_FILES=\
	$(TARFILE_NAME).patch

.IF "$(SYSTEM_POSTGRESQL)" == "YES"
@all:
    @echo "Using system postgresql..."
.ENDIF


.IF "$(GUI)$(COM)"=="WNTMSC"
CONFIGURE_DIR=.
BUILD_DIR=src

CONFIGURE_ACTION =
BUILD_ACTION = nmake -f win32.mak USE_SSL=1 USE_LDAP=1 USE_MOZLDAL=1
.ELSE
CONFIGURE_DIR=.
BUILD_DIR=src/interfaces/libpq

CONFIGURE_ACTION = ./configure --without-readline --disable-shared --with-openssl
.IF "$(WITH_LDAP)" == "YES"
CONFIGURE_ACTION += --with-ldap
.ENDIF
.IF "$(WITH_OPENLDAP)" != "YES"
CONFIGURE_ACTION += --with-mozldap
.ELSE
.ENDIF
BUILD_ACTION = make -j$(GMAKE_MODULE_PARALLELISM) all-static-lib
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

