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

EXT_PROJECT_NAME=postgresql-9.1.1
MAJOR_VER=9.1

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=$(EXT_PROJECT_NAME)
TARFILE_MD5=061a9f17323117c9358ed60f33ecff78

.IF "$(SYSTEM_POSTGRESQL)" == "YES"
@all:
    @echo "Using system postgresql..."
.ENDIF


BUILD_DIR=.
# TODO:
# --datarootdir changes where libpq expects internationalisation of its messages
#               (which we don't install anyway for now...)
# --sysconfdir: config files. Ideally, we would like that to be "the same as the platform default",
#               but that's quite some guessing work.
.IF "$(VERBOSE)"==""
MAKE_SILENT=-s
.ENDIF
BUILD_ACTION = \
            ./configure   --sysconfdir=/etc/postgresql-common \
			  --datarootdir=/usr/share/ \
			  --datadir=/usr/share/postgresql/$(MAJOR_VER) \
			  --bindir=/usr/lib/postgresql/$(MAJOR_VER)/bin \
			  --libdir=/usr/lib/ \
			  --includedir=/usr/include/postgresql/ \
			  --without-readline \
	 && DESTDIR="$(SRC_ROOT)/$(PRJNAME)/$(INPATH)/" \
	 && export DESTDIR \
	 && make $(MAKE_SILENT) -j$(GMAKE_MODULE_PARALLELISM) -C src/interfaces/libpq install \
	 && ./config/install-sh -c -m 644 src/include/postgres_ext.h "$${{DESTDIR}}/usr/include/postgresql/"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

