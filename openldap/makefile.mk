#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=.

PRJNAME=openldap
TARGET=so_openldap

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_OPENLDAP)" == "YES"
all:
	@echo "An already available installation of openldap should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

.IF "$(OS)"=="WNT" || "$(OS)"=="ANDROID" || "$(OS)"=="IOS"
all:
	@echo "Openldap is not needed on Windows, Android or iOS."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=openldap-2.4.31
TARFILE_MD5=804c6cb5698db30b75ad0ff1c25baefd

PATCH_FILES=\
	$(TARFILE_NAME).patch

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--disable-slapd --with-pic --with-tls=moznss --without-cyrus-sasl --disable-shared --enable-static

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --with-yielding_select=yes ac_cv_func_memcmp_working=yes
.ENDIF

.IF "$(SYSTEM_NSS)" == "YES"
CONFIGURE_FLAGS+=CPPFLAGS="$(NSS_CFLAGS)" CFLAGS="$(NSS_CFLAGS)" LDFLAGS="$(NSS_LIBS)"
.ELSE
CONFIGURE_FLAGS+=CPPFLAGS="-I $(SOLARVER)$/$(INPATH)$/inc$/mozilla$/nss -I $(SOLARVER)$/$(INPATH)$/inc$/mozilla$/nspr"
CONFIGURE_FLAGS+=CFLAGS="-I $(SOLARVER)$/$(INPATH)$/inc$/mozilla$/nss -I $(SOLARVER)$/$(INPATH)$/inc$/mozilla$/nspr"
CONFIGURE_FLAGS+=LDFLAGS="-L$(SOLARVER)$/$(INPATH)$/lib"
.ENDIF

BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE)

OUT2INC+=include$/slapi-plugin.h
OUT2INC+=include$/ldap_utf8.h
OUT2INC+=include$/ldap.h
OUT2INC+=include$/ldap_cdefs.h
OUT2INC+=include$/ldap_features.h
OUT2INC+=include$/ldap_schema.h
OUT2INC+=include$/ldif.h
OUT2INC+=include$/lber_types.h
OUT2INC+=include$/lber.h

OUT2LIB+=libraries$/libldap$/.libs$/libldap.a
OUT2LIB+=libraries$/libldap_r$/.libs$/libldap_r.a
OUT2LIB+=libraries$/liblber$/.libs$/liblber.a

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

