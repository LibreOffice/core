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

$(eval $(call gb_Executable_Executable,coolforkit-ns))

$(eval $(call gb_Executable_set_visibility_default,coolforkit-ns))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolforkit-ns,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolforkit-ns,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolforkit-ns))

$(eval $(call gb_Executable_set_include,coolforkit-ns, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
    $(online_poco_inc) \
))

$(eval $(call gb_Executable_use_static_libraries,coolforkit-ns, \
    forkit \
    kitglobals \
    shared \
    simd \
))

$(eval $(call gb_Executable_use_externals,coolforkit-ns, \
    expat \
    libpng \
    openssl \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,coolforkit-ns, \
    -Wl$(COMMA)--whole-archive \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib))) \
    -Wl$(COMMA)--no-whole-archive \
    $(UNIX_DLAPI_LIBS) \
    $(if $(filter LINUX,$(OS)),-lrt) \
    $(ONLINE.CAP_LIBS) \
))
$(call gb_Executable_get_target,coolforkit-ns): \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib)))

ifneq ($(OS),MACOSX)
$(eval $(call gb_Executable_add_ldflags,coolforkit-ns,-Wl$(COMMA)-E))
endif

$(eval $(call gb_Executable_add_generated_exception_objects,coolforkit-ns, \
    kit/forkit-variant-ns \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolforkit-ns,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
