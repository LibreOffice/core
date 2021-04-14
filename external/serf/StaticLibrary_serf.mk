# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,serf))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,serf))

$(eval $(call gb_StaticLibrary_use_unpacked,serf,serf))

$(eval $(call gb_StaticLibrary_use_externals,serf,\
    apr \
    openssl \
))

$(eval $(call gb_StaticLibrary_set_include,serf,\
	-I$(call gb_UnpackedTarball_get_dir,serf) \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,serf,\
    UnpackedTarball/serf/context \
    UnpackedTarball/serf/incoming \
    UnpackedTarball/serf/outgoing \
    UnpackedTarball/serf/ssltunnel \
    UnpackedTarball/serf/auth/auth_basic \
    UnpackedTarball/serf/auth/auth \
    UnpackedTarball/serf/auth/auth_digest \
    UnpackedTarball/serf/auth/auth_spnego \
    UnpackedTarball/serf/auth/auth_spnego_gss \
    UnpackedTarball/serf/auth/auth_spnego_sspi \
    UnpackedTarball/serf/buckets/aggregate_buckets \
    UnpackedTarball/serf/buckets/allocator \
    UnpackedTarball/serf/buckets/barrier_buckets \
    UnpackedTarball/serf/buckets/buckets \
    UnpackedTarball/serf/buckets/bwtp_buckets \
    UnpackedTarball/serf/buckets/deflate_buckets \
    UnpackedTarball/serf/buckets/dechunk_buckets \
    UnpackedTarball/serf/buckets/file_buckets \
    UnpackedTarball/serf/buckets/headers_buckets \
    UnpackedTarball/serf/buckets/chunk_buckets \
    UnpackedTarball/serf/buckets/iovec_buckets \
    UnpackedTarball/serf/buckets/limit_buckets \
    UnpackedTarball/serf/buckets/mmap_buckets \
    UnpackedTarball/serf/buckets/request_buckets \
    UnpackedTarball/serf/buckets/response_body_buckets \
    UnpackedTarball/serf/buckets/response_buckets \
    UnpackedTarball/serf/buckets/simple_buckets \
    UnpackedTarball/serf/buckets/socket_buckets \
    UnpackedTarball/serf/buckets/ssl_buckets \
))

# vim: set noet sw=4 ts=4:
