# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,shared))

$(eval $(call gb_StaticLibrary_set_visibility_default,shared))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,shared,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,shared,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,shared))

$(eval $(call gb_StaticLibrary_set_include,shared, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    -I$(online_srcdir)/common \
    -I$(online_srcdir)/net \
    -I$(online_srcdir)/wsd \
    -I$(online_srcdir)/kit \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_StaticLibrary_use_externals,shared, \
    expat \
    libpng \
    openssl_headers \
    zlib \
    zstd \
))

# Set in Makefile.am's build-fuzzer-gbuild-libs target.
$(eval $(call gb_StaticLibrary_add_cxxflags,shared, \
    $(ONLINE.FUZZER_CFLAGS) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,shared, \
    common/AIHttpTransport \
    common/Authorization \
    common/CommandControl \
    common/ConfigUtil \
    common/CoolMount \
    common/Crypto \
    common/FileUtil \
    common/FileUtil-unix \
    common/JailUtil \
    common/Landlock \
    common/Log-common \
    common/Log-poco \
    common/MobileApp \
    common/ProcUtil \
    common/Protocol \
    common/RegexUtil \
    common/Seccomp \
    common/Session \
    common/SigUtil-server \
    common/Simd \
    common/SpookyV2 \
    common/StringVector \
    common/Syscall \
    common/TraceEvent \
    common/Unit \
    common/Unit-server \
    common/Uri \
    common/Util \
    common/Util-linux \
    common/Util-server \
    common/Util-unix \
    kit/KitQueue \
    kit/LogUI \
    net/FakeSocket-stub \
    net/HttpHelper \
    net/HttpRequest \
    net/NetUtil \
    net/Socket \
    net/Ssl \
    wsd/Exceptions \
))

# vim: set noet sw=4 ts=4:
