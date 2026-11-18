# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,forkitmain))

$(eval $(call gb_StaticLibrary_set_visibility_default,forkitmain))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,forkitmain,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,forkitmain,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,forkitmain))

$(eval $(call gb_StaticLibrary_set_include,forkitmain, \
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

$(eval $(call gb_StaticLibrary_use_externals,forkitmain, \
    expat \
    libpng \
    openssl_headers \
    zlib \
    zstd \
))

# kit/forkit-main holds main() and the process-wide globals of the forkit executables.
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,forkitmain, \
    kit/forkit-main \
))

# vim: set noet sw=4 ts=4:
