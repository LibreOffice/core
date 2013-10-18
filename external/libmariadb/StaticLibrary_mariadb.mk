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
$(eval $(call gb_StaticLibrary_add_cflags,mariadblib,-DHAVE_DLOPEN))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,mariadblib,\
    UnpackedTarball/mariadb/libmysql/array \
    UnpackedTarball/mariadb/libmysql/bchange \
    UnpackedTarball/mariadb/libmysql/bmove \
    UnpackedTarball/mariadb/libmysql/bmove_upp \
    UnpackedTarball/mariadb/libmysql/charset \
    UnpackedTarball/mariadb/libmysql/client_plugin \
    UnpackedTarball/mariadb/libmysql/dbug \
    UnpackedTarball/mariadb/libmysql/default \
    UnpackedTarball/mariadb/libmysql/errmsg \
    UnpackedTarball/mariadb/libmysql/errors \
    UnpackedTarball/mariadb/libmysql/getopt \
    UnpackedTarball/mariadb/libmysql/getopt1 \
    UnpackedTarball/mariadb/libmysql/get_password \
    UnpackedTarball/mariadb/libmysql/int2str \
    UnpackedTarball/mariadb/libmysql/is_prefix \
    UnpackedTarball/mariadb/libmysql/libmysql \
    UnpackedTarball/mariadb/libmysql/list \
    UnpackedTarball/mariadb/libmysql/llstr \
    UnpackedTarball/mariadb/libmysql/longlong2str \
    UnpackedTarball/mariadb/libmysql/mf_dirname \
    UnpackedTarball/mariadb/libmysql/mf_fn_ext \
    UnpackedTarball/mariadb/libmysql/mf_format \
    UnpackedTarball/mariadb/libmysql/mf_loadpath \
    UnpackedTarball/mariadb/libmysql/mf_pack \
    UnpackedTarball/mariadb/libmysql/mf_path \
    UnpackedTarball/mariadb/libmysql/mf_tempfile \
    UnpackedTarball/mariadb/libmysql/mf_unixpath \
    UnpackedTarball/mariadb/libmysql/mf_wcomp \
    UnpackedTarball/mariadb/libmysql/mulalloc \
    UnpackedTarball/mariadb/libmysql/my_alloc \
    UnpackedTarball/mariadb/libmysql/my_auth \
    UnpackedTarball/mariadb/libmysql/my_charset \
    UnpackedTarball/mariadb/libmysql/my_compress \
    UnpackedTarball/mariadb/libmysql/my_create \
    UnpackedTarball/mariadb/libmysql/my_delete \
    UnpackedTarball/mariadb/libmysql/my_div \
    UnpackedTarball/mariadb/libmysql/my_error \
    UnpackedTarball/mariadb/libmysql/my_fopen \
    UnpackedTarball/mariadb/libmysql/my_fstream \
    UnpackedTarball/mariadb/libmysql/my_gethostbyname \
    UnpackedTarball/mariadb/libmysql/my_getwd \
    UnpackedTarball/mariadb/libmysql/my_init \
    UnpackedTarball/mariadb/libmysql/my_lib \
    UnpackedTarball/mariadb/libmysql/my_loaddata \
    UnpackedTarball/mariadb/libmysql/my_malloc \
    UnpackedTarball/mariadb/libmysql/my_messnc \
    UnpackedTarball/mariadb/libmysql/my_net \
    UnpackedTarball/mariadb/libmysql/my_once \
    UnpackedTarball/mariadb/libmysql/my_open \
    UnpackedTarball/mariadb/libmysql/my_port \
    UnpackedTarball/mariadb/libmysql/my_pthread \
    UnpackedTarball/mariadb/libmysql/my_read \
    UnpackedTarball/mariadb/libmysql/my_realloc \
    UnpackedTarball/mariadb/libmysql/my_secure \
    UnpackedTarball/mariadb/libmysql/my_seek \
    UnpackedTarball/mariadb/libmysql/my_static \
    UnpackedTarball/mariadb/libmysql/my_stmt \
    UnpackedTarball/mariadb/libmysql/my_stmt_codec \
    UnpackedTarball/mariadb/libmysql/my_symlink \
    UnpackedTarball/mariadb/libmysql/my_thr_init \
    UnpackedTarball/mariadb/libmysql/my_vsnprintf \
    UnpackedTarball/mariadb/libmysql/my_write \
    UnpackedTarball/mariadb/libmysql/net \
    UnpackedTarball/mariadb/libmysql/password \
    UnpackedTarball/mariadb/libmysql/safemalloc \
    UnpackedTarball/mariadb/libmysql/sha1 \
    UnpackedTarball/mariadb/libmysql/str2int \
    UnpackedTarball/mariadb/libmysql/strcend \
    UnpackedTarball/mariadb/libmysql/strcont \
    UnpackedTarball/mariadb/libmysql/strend \
    UnpackedTarball/mariadb/libmysql/strfill \
    UnpackedTarball/mariadb/libmysql/string \
    UnpackedTarball/mariadb/libmysql/strinstr \
    UnpackedTarball/mariadb/libmysql/strmake \
    UnpackedTarball/mariadb/libmysql/strmov \
    UnpackedTarball/mariadb/libmysql/strnlen \
    UnpackedTarball/mariadb/libmysql/strnmov \
    UnpackedTarball/mariadb/libmysql/strto \
    UnpackedTarball/mariadb/libmysql/strtoll \
    UnpackedTarball/mariadb/libmysql/strtoull \
    UnpackedTarball/mariadb/libmysql/strxmov \
    UnpackedTarball/mariadb/libmysql/strxnmov \
    UnpackedTarball/mariadb/libmysql/thr_mutex \
    UnpackedTarball/mariadb/libmysql/typelib \
    UnpackedTarball/mariadb/libmysql/violite \
))

# vim: set noet sw=4 ts=4:
