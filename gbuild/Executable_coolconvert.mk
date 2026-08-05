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

$(eval $(call gb_Executable_Executable,coolconvert))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolconvert,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolconvert,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolconvert))

$(eval $(call gb_Executable_set_include,coolconvert, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
    $(online_poco_inc) \
))

$(eval $(call gb_Executable_use_static_libraries,coolconvert, \
    PocoCrypto \
    PocoFoundation \
    PocoJSON \
    PocoNet \
    PocoNetSSL \
    PocoUtil \
    PocoXML \
))

$(eval $(call gb_Executable_use_externals,coolconvert, \
    expat \
    openssl \
    zlib \
))

$(eval $(call gb_Executable_add_generated_exception_objects,coolconvert, \
    tools/Tool \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolconvert,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
