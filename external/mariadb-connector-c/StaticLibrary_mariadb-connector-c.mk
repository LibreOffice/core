# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,mariadb-connector-c))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,mariadb-connector-c))

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
	UnpackedTarball/mariadb-connector-c/libmariadb/array \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_dyncol \
	UnpackedTarball/mariadb-connector-c/libmariadb/bchange \
	UnpackedTarball/mariadb-connector-c/libmariadb/bmove \
	UnpackedTarball/mariadb-connector-c/libmariadb/bmove_upp \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_charset \
	UnpackedTarball/mariadb-connector-c/libmariadb/hash \
	UnpackedTarball/mariadb-connector-c/libmariadb/violite \
	UnpackedTarball/mariadb-connector-c/libmariadb/net \
	UnpackedTarball/mariadb-connector-c/libmariadb/charset \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_time \
	UnpackedTarball/mariadb-connector-c/libmariadb/dbug \
	UnpackedTarball/mariadb-connector-c/libmariadb/default \
	UnpackedTarball/mariadb-connector-c/libmariadb/errmsg \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_vsnprintf \
	UnpackedTarball/mariadb-connector-c/libmariadb/errors \
	UnpackedTarball/mariadb-connector-c/libmariadb/getopt1 \
	UnpackedTarball/mariadb-connector-c/libmariadb/getopt \
	UnpackedTarball/mariadb-connector-c/libmariadb/get_password \
	UnpackedTarball/mariadb-connector-c/libmariadb/int2str \
	UnpackedTarball/mariadb-connector-c/libmariadb/is_prefix \
	UnpackedTarball/mariadb-connector-c/libmariadb/libmariadb \
	UnpackedTarball/mariadb-connector-c/libmariadb/list \
	UnpackedTarball/mariadb-connector-c/libmariadb/llstr \
	UnpackedTarball/mariadb-connector-c/libmariadb/longlong2str \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_dirname \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_fn_ext \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_format \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_loadpath \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_pack \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_path \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_unixpath \
	UnpackedTarball/mariadb-connector-c/libmariadb/mf_wcomp \
	UnpackedTarball/mariadb-connector-c/libmariadb/mulalloc \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_alloc \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_compress \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_div \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_error \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_fopen \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_fstream \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_getwd \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_init \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_lib \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_malloc \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_messnc \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_net \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_once \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_open \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_port \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_pthread \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_read \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_realloc \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_seek \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_static \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_symlink \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_thr_init \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_write \
	UnpackedTarball/mariadb-connector-c/libmariadb/password \
	UnpackedTarball/mariadb-connector-c/libmariadb/str2int \
	UnpackedTarball/mariadb-connector-c/libmariadb/strcend \
	UnpackedTarball/mariadb-connector-c/libmariadb/strcont \
	UnpackedTarball/mariadb-connector-c/libmariadb/strend \
	UnpackedTarball/mariadb-connector-c/libmariadb/strfill \
	UnpackedTarball/mariadb-connector-c/libmariadb/string \
	UnpackedTarball/mariadb-connector-c/libmariadb/strinstr \
	UnpackedTarball/mariadb-connector-c/libmariadb/strmake \
	UnpackedTarball/mariadb-connector-c/libmariadb/strmov \
	UnpackedTarball/mariadb-connector-c/libmariadb/strnmov \
	UnpackedTarball/mariadb-connector-c/libmariadb/strtoll \
	UnpackedTarball/mariadb-connector-c/libmariadb/strtoull \
	UnpackedTarball/mariadb-connector-c/libmariadb/strxmov \
	UnpackedTarball/mariadb-connector-c/libmariadb/strxnmov \
	UnpackedTarball/mariadb-connector-c/libmariadb/thr_mutex \
	UnpackedTarball/mariadb-connector-c/libmariadb/typelib \
	UnpackedTarball/mariadb-connector-c/libmariadb/sha1 \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_stmt \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_loaddata \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_stmt_codec \
	UnpackedTarball/mariadb-connector-c/libmariadb/client_plugin \
	UnpackedTarball/mariadb-connector-c/libmariadb/my_auth \
	UnpackedTarball/mariadb-connector-c/libmariadb/ma_secure \
$(if $(filter $(OS),WNT), \
	UnpackedTarball/mariadb-connector-c/win-iconv/win_iconv,) \
))

# vim: set noet sw=4 ts=4:
