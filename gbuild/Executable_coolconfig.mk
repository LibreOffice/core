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

$(eval $(call gb_Executable_Executable,coolconfig))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolconfig,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolconfig,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolconfig))

$(eval $(call gb_Executable_set_include,coolconfig, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_Executable_use_static_libraries,coolconfig, \
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

$(eval $(call gb_Executable_use_externals,coolconfig, \
    expat \
    openssl \
    openssl_headers \
))

$(eval $(call gb_Executable_add_libs,coolconfig,$(UNIX_DLAPI_LIBS)))

$(eval $(call gb_Executable_add_generated_exception_objects,coolconfig, \
    common/Crypto \
    tools/Config \
    tools/ConfigMigrationAssistant \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolconfig,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
