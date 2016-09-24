# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,mariadblib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,mariadblib))

$(eval $(call gb_StaticLibrary_use_unpacked,mariadblib,mariadb))

$(eval $(call gb_StaticLibrary_set_include,mariadblib,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,mariadb)/include \
))

# This is needed for MSVC 2008: it somehow finds a dlopen somewhere
# but the static library then contains unreferenced symbols.
# This macro enables a re-definition to native Win32 APIs in my_global.h.
## TODO missing enable: -DHAVE_OPENSSL -D HAVE_COMPRESS
## (but then need to add "-lssl -lcrypto -lz" to mysqlcppconn linking)
$(eval $(call gb_StaticLibrary_add_cflags,mariadblib,-DHAVE_DLOPEN -D ENABLED_LOCAL_INFILE -D LIBMARIADB -D THREAD -DSQLITE_ENABLE_COLUMN_METADATA=1))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,mariadblib,\
	UnpackedTarball/mariadb/libmariadb/array \
	UnpackedTarball/mariadb/libmariadb/ma_dyncol \
	UnpackedTarball/mariadb/libmariadb/bchange \
	UnpackedTarball/mariadb/libmariadb/bmove \
	UnpackedTarball/mariadb/libmariadb/bmove_upp \
	UnpackedTarball/mariadb/libmariadb/my_charset \
	UnpackedTarball/mariadb/libmariadb/hash \
	UnpackedTarball/mariadb/libmariadb/violite \
	UnpackedTarball/mariadb/libmariadb/net \
	UnpackedTarball/mariadb/libmariadb/charset \
	UnpackedTarball/mariadb/libmariadb/ma_time \
	UnpackedTarball/mariadb/libmariadb/dbug \
	UnpackedTarball/mariadb/libmariadb/default \
	UnpackedTarball/mariadb/libmariadb/errmsg \
	UnpackedTarball/mariadb/libmariadb/my_vsnprintf \
	UnpackedTarball/mariadb/libmariadb/errors \
	UnpackedTarball/mariadb/libmariadb/getopt1 \
	UnpackedTarball/mariadb/libmariadb/getopt \
	UnpackedTarball/mariadb/libmariadb/get_password \
	UnpackedTarball/mariadb/libmariadb/int2str \
	UnpackedTarball/mariadb/libmariadb/is_prefix \
	UnpackedTarball/mariadb/libmariadb/libmariadb \
	UnpackedTarball/mariadb/libmariadb/list \
	UnpackedTarball/mariadb/libmariadb/llstr \
	UnpackedTarball/mariadb/libmariadb/longlong2str \
	UnpackedTarball/mariadb/libmariadb/mf_dirname \
	UnpackedTarball/mariadb/libmariadb/mf_fn_ext \
	UnpackedTarball/mariadb/libmariadb/mf_format \
	UnpackedTarball/mariadb/libmariadb/mf_loadpath \
	UnpackedTarball/mariadb/libmariadb/mf_pack \
	UnpackedTarball/mariadb/libmariadb/mf_path \
	UnpackedTarball/mariadb/libmariadb/mf_unixpath \
	UnpackedTarball/mariadb/libmariadb/mf_wcomp \
	UnpackedTarball/mariadb/libmariadb/mulalloc \
	UnpackedTarball/mariadb/libmariadb/my_alloc \
	UnpackedTarball/mariadb/libmariadb/my_compress \
	UnpackedTarball/mariadb/libmariadb/my_div \
	UnpackedTarball/mariadb/libmariadb/my_error \
	UnpackedTarball/mariadb/libmariadb/my_fopen \
	UnpackedTarball/mariadb/libmariadb/my_fstream \
	UnpackedTarball/mariadb/libmariadb/my_getwd \
	UnpackedTarball/mariadb/libmariadb/my_init \
	UnpackedTarball/mariadb/libmariadb/my_lib \
	UnpackedTarball/mariadb/libmariadb/my_malloc \
	UnpackedTarball/mariadb/libmariadb/my_messnc \
	UnpackedTarball/mariadb/libmariadb/my_net \
	UnpackedTarball/mariadb/libmariadb/my_once \
	UnpackedTarball/mariadb/libmariadb/my_open \
	UnpackedTarball/mariadb/libmariadb/my_port \
	UnpackedTarball/mariadb/libmariadb/my_pthread \
	UnpackedTarball/mariadb/libmariadb/my_read \
	UnpackedTarball/mariadb/libmariadb/my_realloc \
	UnpackedTarball/mariadb/libmariadb/my_seek \
	UnpackedTarball/mariadb/libmariadb/my_static \
	UnpackedTarball/mariadb/libmariadb/my_symlink \
	UnpackedTarball/mariadb/libmariadb/my_thr_init \
	UnpackedTarball/mariadb/libmariadb/my_write \
	UnpackedTarball/mariadb/libmariadb/password \
	UnpackedTarball/mariadb/libmariadb/str2int \
	UnpackedTarball/mariadb/libmariadb/strcend \
	UnpackedTarball/mariadb/libmariadb/strcont \
	UnpackedTarball/mariadb/libmariadb/strend \
	UnpackedTarball/mariadb/libmariadb/strfill \
	UnpackedTarball/mariadb/libmariadb/string \
	UnpackedTarball/mariadb/libmariadb/strinstr \
	UnpackedTarball/mariadb/libmariadb/strmake \
	UnpackedTarball/mariadb/libmariadb/strmov \
	UnpackedTarball/mariadb/libmariadb/strnmov \
	UnpackedTarball/mariadb/libmariadb/strtoll \
	UnpackedTarball/mariadb/libmariadb/strtoull \
	UnpackedTarball/mariadb/libmariadb/strxmov \
	UnpackedTarball/mariadb/libmariadb/strxnmov \
	UnpackedTarball/mariadb/libmariadb/thr_mutex \
	UnpackedTarball/mariadb/libmariadb/typelib \
	UnpackedTarball/mariadb/libmariadb/sha1 \
	UnpackedTarball/mariadb/libmariadb/my_stmt \
	UnpackedTarball/mariadb/libmariadb/my_loaddata \
	UnpackedTarball/mariadb/libmariadb/my_stmt_codec \
	UnpackedTarball/mariadb/libmariadb/client_plugin \
	UnpackedTarball/mariadb/libmariadb/my_auth \
	UnpackedTarball/mariadb/libmariadb/ma_secure \
))

# vim: set noet sw=4 ts=4:
