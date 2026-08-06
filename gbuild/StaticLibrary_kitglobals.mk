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

$(eval $(call gb_StaticLibrary_StaticLibrary,kitglobals))

$(eval $(call gb_StaticLibrary_set_visibility_default,kitglobals))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,kitglobals,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,kitglobals,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,kitglobals))

$(eval $(call gb_StaticLibrary_set_include,kitglobals, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_StaticLibrary_use_externals,kitglobals, \
    openssl_headers \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,kitglobals, \
    kit/KitGlobals \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_StaticLibrary_add_defs,kitglobals,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
