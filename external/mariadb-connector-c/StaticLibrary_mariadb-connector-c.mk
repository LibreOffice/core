# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,mariadb-connector-c))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,mariadb-connector-c))

$(eval $(call gb_StaticLibrary_use_unpacked,mariadb-connector-c,mariadb-connector-c))

$(eval $(call gb_StaticLibrary_set_include,mariadb-connector-c,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,mariadb-connector-c)/include \
))

# This is needed for MSVC 2008: it somehow finds a dlopen somewhere
# but the static library then contains unreferenced symbols.
# This macro enables a re-definition to native Win32 APIs in my_global.h.
## TODO missing enable: -DHAVE_OPENSSL -D HAVE_COMPRESS
## (but then need to add "-lssl -lcrypto -lz" to mysqlcppconn linking)
$(eval $(call gb_StaticLibrary_add_cflags,mariadb-connector-c,-DHAVE_DLOPEN -D ENABLED_LOCAL_INFILE -D LIBMARIADB -D THREAD -DSQLITE_ENABLE_COLUMN_METADATA=1))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_cflags,mariadb-connector-c,-D_TIMESPEC_DEFINED -DHAVE_STRTOULL))
$(eval $(call gb_StaticLibrary_set_include,mariadb-connector-c,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,mariadb-connector-c)/win-iconv \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,mariadb-connector-c,\
	UnpackedTarball/mariadb-connector-c/libmariadb/bmove_upp \
	UnpackedTarball/mariadb-connector-c/libmariadb/get_password \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_alloc \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_array \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_charset \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_compress \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_context \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_default \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_dtoa \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_errmsg \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_hash \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_init \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_io \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_list \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_ll2str \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_loaddata \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_net \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_password \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_pvio \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_sha1 \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_stmt_codec \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_string \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_time \
	UnpackedTarball/mariadb-connector-c/libmariadb/mariadb_async \
	UnpackedTarball/mariadb-connector-c/libmariadb/mariadb_charset \
	UnpackedTarball/mariadb-connector-c/libmariadb/mariadb_dyncol \
	UnpackedTarball/mariadb-connector-c/libmariadb/mariadb_lib \
	UnpackedTarball/mariadb-connector-c/libmariadb/mariadb_stmt \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_client_plugin \
	UnpackedTarball/mariadb-connector-c/plugins/auth/my_auth \
	UnpackedTarball/mariadb-connector-c/plugins/auth/caching_sha2_pw \
	UnpackedTarball/mariadb-connector-c/plugins/pvio/pvio_socket \
	$(if $(filter $(OS),WNT), \
		UnpackedTarball/mariadb-connector-c/libmariadb/win32_errmsg \
		UnpackedTarball/mariadb-connector-c/libmariadb/secure/win_crypt \
		UnpackedTarball/mariadb-connector-c/win-iconv/win_iconv) \
))

# vim: set noet sw=4 ts=4:
