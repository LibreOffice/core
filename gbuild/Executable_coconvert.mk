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

$(eval $(call gb_Executable_Executable,coconvert))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coconvert,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coconvert,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coconvert))

$(eval $(call gb_Executable_set_include,coconvert, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    -I$(SRCDIR)/include \
    $$(INCLUDE) \
    $(online_poco_inc) \
))

$(eval $(call gb_Executable_use_static_libraries,coconvert, \
    PocoCrypto \
    PocoFoundation \
    PocoJSON \
    PocoNet \
    PocoNetSSL \
    PocoUtil \
    PocoXML \
))

$(eval $(call gb_Executable_use_externals,coconvert, \
    expat \
    openssl \
    zlib \
))

$(eval $(call gb_Executable_add_libs,coconvert,$(UNIX_DLAPI_LIBS)))

$(eval $(call gb_Executable_add_generated_exception_objects,coconvert, \
    tools/coconvert \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coconvert,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
