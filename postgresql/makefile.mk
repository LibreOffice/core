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
	postgresql-moz-ldap.patch \
	$(TARFILE_NAME)-libreoffice.patch \
	$(TARFILE_NAME)-autoreconf.patch \
    postgresql-9.1.1-configure.patch
# postgresql-9.1.1-configure.patch: move check for with_krb5 before with_gssapi,
#  so that LIBS does not already contain -lgssapi_krb5 (which links against
#  -lkrb5) when looking for symbols from -lkrb5, and within with_krb5, move
#  com_err before krb5, so that looking with symbol com_err does not erroneously
#  pick -lkrb5 (which links against -lcom_err) instead of -lcom_err

.IF "$(SYSTEM_POSTGRESQL)" == "YES"
@all:
    @echo "Using system postgresql..."
.ENDIF


.IF "$(GUI)$(COM)"=="WNTMSC"
CONFIGURE_DIR=.
BUILD_DIR=src

CONFIGURE_ACTION =
SOLARINC += -I$(SOLARVER)$/$(INPATH)$/inc$/mozilla$/ldap
BUILD_ACTION = nmake -f win32.mak USE_SSL=1 USE_LDAP=1 USE_MOZLDAP=1
.ELSE
CONFIGURE_DIR=.
BUILD_DIR=src/interfaces/libpq

CONFIGURE_ACTION = CPPFLAGS="$(SOLARINC)" LDFLAGS="$(SOLARLIB)" ./configure --without-readline --disable-shared --with-openssl

.IF "$(CROSS_COMPILING)" == "YES"
CONFIGURE_ACTION += --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.IF "$(WITH_LDAP)" == "YES"
CONFIGURE_ACTION += --with-ldap
.IF "$(WITH_OPENLDAP)" != "YES"
CONFIGURE_ACTION += --with-includes='$(SOLARVER)$/$(INPATH)$/inc$/mozilla$/ldap' --with-mozldap
.ENDIF
.ENDIF # "$(WITH_LDAP)" == "YES"

.IF "$(WITH_KRB5)" == "YES"
CONFIGURE_ACTION += --with-krb5
.ENDIF
.IF "$(WITH_GSSAPI)" == "YES"
CONFIGURE_ACTION += --with-gssapi
.ENDIF

BUILD_ACTION = make -j$(GMAKE_MODULE_PARALLELISM) all-static-lib libpq-flags.mk

.ENDIF # "$(GUI)$(COM)"=="WNTMSC"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

