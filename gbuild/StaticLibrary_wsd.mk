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

$(eval $(call gb_StaticLibrary_StaticLibrary,wsd))

$(eval $(call gb_StaticLibrary_set_visibility_default,wsd))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,wsd,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,wsd,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,wsd))

$(eval $(call gb_StaticLibrary_set_include,wsd, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    -I$(online_srcdir)/common \
    -I$(online_srcdir)/net \
    -I$(online_srcdir)/wsd \
    -I$(online_srcdir)/kit \
    -I$(SRCDIR)/include \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_StaticLibrary_use_externals,wsd, \
    expat \
    libpng \
    openssl \
    openssl_headers \
    zlib \
    zstd \
))

# Set in Makefile.am's build-fuzzer-gbuild-libs target.
$(eval $(call gb_StaticLibrary_add_cxxflags,wsd, \
    $(ONLINE.FUZZER_CFLAGS) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,wsd, \
    wsd/AIChatSession \
    wsd/AIUtil \
    wsd/Admin \
    wsd/AdminModel \
    wsd/Auth \
    wsd/CacheUtil \
    wsd/ClientRequestDispatcher \
    wsd/ClientSession \
    wsd/CollabBroker \
    wsd/CollabFileProxy \
    wsd/CollabSocketHandler \
    wsd/ContentType \
    wsd/DeckSpec \
    wsd/DocumentBroker \
    wsd/FileServer \
    wsd/FileServerUtil \
    wsd/HealthCheck \
    wsd/HostUtil \
    wsd/ProofKey \
    wsd/ProxyProtocol \
    wsd/ProxyRequestHandler \
    wsd/QuarantineUtil \
    wsd/RemoteConfig \
    wsd/RequestDetails \
    wsd/RequestVettingStation \
    wsd/ServerAuditUtil \
    wsd/SlideCache \
    wsd/SpecialBrokers \
    wsd/Storage \
    wsd/TileCache \
    wsd/Unzip \
    wsd/dumpWsdState \
    wsd/wopi/CheckFileInfo \
    wsd/wopi/StorageConnectionManager \
    wsd/wopi/WopiProxy \
    wsd/wopi/WopiStorage \
))

# vim: set noet sw=4 ts=4:
