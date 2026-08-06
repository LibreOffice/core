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

$(eval $(call gb_Executable_Executable,lokitclient))

$(eval $(call gb_Executable_set_generated_cxx_suffix,lokitclient,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,lokitclient,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,lokitclient))

$(eval $(call gb_Executable_set_include,lokitclient, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    -I$(online_srcdir)/common \
    -I$(online_srcdir)/net \
    -I$(online_srcdir)/wsd \
    -I$(online_srcdir)/kit \
    $$(INCLUDE) \
    $(online_poco_inc) \
))

$(eval $(call gb_Executable_use_static_libraries,lokitclient, \
    PocoCrypto \
    PocoFoundation \
    PocoJSON \
    PocoNet \
    PocoNetSSL \
    PocoUtil \
    PocoXML \
    dummytraceevent \
    globals \
    shared \
))

$(eval $(call gb_Executable_use_externals,lokitclient, \
    expat \
    libpng \
    openssl \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,lokitclient,$(UNIX_DLAPI_LIBS)))

$(eval $(call gb_Executable_add_generated_exception_objects,lokitclient, \
    tools/KitClient \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,lokitclient,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
