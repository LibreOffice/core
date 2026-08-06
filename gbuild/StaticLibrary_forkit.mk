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

$(eval $(call gb_StaticLibrary_StaticLibrary,forkit))

$(eval $(call gb_StaticLibrary_set_visibility_default,forkit))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,forkit,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,forkit,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,forkit))

$(eval $(call gb_StaticLibrary_set_include,forkit, \
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

$(eval $(call gb_StaticLibrary_use_externals,forkit, \
    expat \
    libpng \
    openssl_headers \
    zlib \
    zstd \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,forkit, \
    kit/ChildSession \
    kit/ForKit \
    kit/Kit \
    kit/KitWebSocket \
    kit/forkit-main \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_StaticLibrary_add_defs,forkit,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
