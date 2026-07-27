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

$(eval $(call gb_Executable_Executable,coolstress))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolstress,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolstress,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolstress))

$(eval $(call gb_Executable_set_include,coolstress, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    -I$(SRCDIR)/../common \
    -I$(SRCDIR)/../net \
    -I$(SRCDIR)/../wsd \
    -I$(SRCDIR)/../kit \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_Executable_use_static_libraries,coolstress, \
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

$(eval $(call gb_Executable_use_externals,coolstress, \
    expat \
    libpng \
    openssl \
    openssl_headers \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,coolstress,$(UNIX_DLAPI_LIBS)))

# The stress tool defaults its test documents to the in-tree test/data:
$(eval $(call gb_Executable_add_defs,coolstress, \
    -DTDOC=\"$(SRCDIR)/../test/data\" \
))

$(eval $(call gb_Executable_add_generated_exception_objects,coolstress, \
    tools/Stress \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolstress,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
