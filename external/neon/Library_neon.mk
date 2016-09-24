# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,neon))

$(eval $(call gb_Library_use_unpacked,neon,neon))

$(eval $(call gb_Library_use_externals,neon,\
	gnutls \
	libxml2 \
	libgcrypt \
	openssl \
	openssl_headers \
	zlib \
))

$(eval $(call gb_Library_set_visibility_default,neon))

# neon has its share of warning... let's spare use
# the pointless spamming
$(eval $(call gb_Library_add_cflags,neon,\
	-w \
))

$(eval $(call gb_Library_set_warnings_not_errors,neon))

$(eval $(call gb_Library_add_generated_cobjects,neon,\
	UnpackedTarball/neon/src/ne_207 \
	UnpackedTarball/neon/src/ne_acl3744 \
	UnpackedTarball/neon/src/ne_alloc \
	UnpackedTarball/neon/src/ne_auth \
	UnpackedTarball/neon/src/ne_basic \
	UnpackedTarball/neon/src/ne_compress \
	UnpackedTarball/neon/src/ne_dates \
	UnpackedTarball/neon/src/ne_i18n \
	UnpackedTarball/neon/src/ne_locks \
	UnpackedTarball/neon/src/ne_md5 \
	UnpackedTarball/neon/src/ne_props \
	UnpackedTarball/neon/src/ne_redirect \
	UnpackedTarball/neon/src/ne_request \
	UnpackedTarball/neon/src/ne_session \
	UnpackedTarball/neon/src/ne_socket \
	UnpackedTarball/neon/src/ne_socks \
	UnpackedTarball/neon/src/ne_string \
	UnpackedTarball/neon/src/ne_uri \
	UnpackedTarball/neon/src/ne_utils \
	UnpackedTarball/neon/src/ne_xml \
	UnpackedTarball/neon/src/ne_xmlreq \
))


ifneq ($(DISABLE_OPENSSL),TRUE)
$(eval $(call gb_Library_add_generated_cobjects,neon,\
	UnpackedTarball/neon/src/ne_openssl \
))
else
$(eval $(call gb_Library_add_generated_cobjects,neon,\
	UnpackedTarball/neon/src/ne_gnutls \
))
endif


ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,neon,\
	ws2_32 \
	crypt32 \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,neon,\
	/DEF:$(SRCDIR)/external/neon/neon.def \
))
endif

$(eval $(call gb_Library_add_generated_cobjects,neon,\
	UnpackedTarball/neon/src/ne_sspi \
))

else
ifneq ($(DISABLE_OPENSSL),TRUE)
$(eval $(call gb_Library_add_generated_cobjects,neon,\
	UnpackedTarball/neon/src/ne_ntlm \
))
endif

endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,neon,\
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
