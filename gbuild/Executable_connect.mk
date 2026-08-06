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

$(eval $(call gb_Executable_Executable,connect))

$(eval $(call gb_Executable_set_generated_cxx_suffix,connect,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,connect,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,connect))

$(eval $(call gb_Executable_set_include,connect, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    -I$(online_srcdir)/common \
    -I$(online_srcdir)/net \
    -I$(online_srcdir)/wsd \
    -I$(online_srcdir)/kit \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_Executable_use_static_libraries,connect, \
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

$(eval $(call gb_Executable_use_externals,connect, \
    expat \
    libpng \
    openssl \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,connect,$(UNIX_DLAPI_LIBS)))

$(eval $(call gb_Executable_add_generated_exception_objects,connect, \
    tools/Connect \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,connect,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
