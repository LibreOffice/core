# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,poco))

$(eval $(call gb_Module_add_targets,poco,\
    UnpackedTarball_poco \
    StaticLibrary_PocoFoundation \
    StaticLibrary_PocoXML \
    StaticLibrary_PocoJSON \
    StaticLibrary_PocoUtil \
    StaticLibrary_PocoNet \
))

# PocoZip is only consumed by wsd, which is server-only; skip it on the app targets that never link
# wsd/Unzip.cpp:
ifeq ($(filter ANDROID iOS EMSCRIPTEN WNT MACOSX,$(OS)),)
$(eval $(call gb_Module_add_targets,poco,\
    StaticLibrary_PocoZip \
))
endif

# PocoCrypto and PocoNetSSL need OpenSSL and are only used by the COOL server
# (SSL) build.  None of the CODA app targets (Android, iOS, Emscripten, Windows,
# macOS) build with SSL or link them, and some ship a reduced OpenSSL (e.g.
# wasm's lacks OCSP, which PocoNetSSL uses), so they are skipped there; they are
# built only on the server platforms (Linux/FreeBSD/...) that have OpenSSL.
ifeq ($(ENABLE_OPENSSL),TRUE)
ifeq ($(filter ANDROID iOS EMSCRIPTEN WNT MACOSX,$(OS)),)
$(eval $(call gb_Module_add_targets,poco,\
    StaticLibrary_PocoCrypto \
    StaticLibrary_PocoNetSSL \
))
endif
endif

# vim: set noet sw=4 ts=4:
