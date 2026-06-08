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

$(eval $(call gb_Executable_Executable,coolmap))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolmap,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolmap,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolmap))

$(eval $(call gb_Executable_set_include,coolmap, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_Executable_use_static_libraries,coolmap, \
    PocoCrypto \
    PocoFoundation \
    PocoJSON \
    PocoNet \
    PocoNetSSL \
    PocoUtil \
    PocoXML \
))

$(eval $(call gb_Executable_use_externals,coolmap, \
    expat \
    openssl \
    zlib \
))

$(eval $(call gb_Executable_add_generated_exception_objects,coolmap, \
    tools/map \
))

# vim: set noet sw=4 ts=4:
