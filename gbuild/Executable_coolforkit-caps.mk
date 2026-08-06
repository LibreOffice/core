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

$(eval $(call gb_Executable_Executable,coolforkit-caps))

$(eval $(call gb_Executable_set_visibility_default,coolforkit-caps))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolforkit-caps,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolforkit-caps,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolforkit-caps))

$(eval $(call gb_Executable_set_include,coolforkit-caps, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_Executable_use_static_libraries,coolforkit-caps, \
    forkit \
    kitglobals \
    shared \
    simd \
))

$(eval $(call gb_Executable_use_externals,coolforkit-caps, \
    expat \
    libpng \
    openssl \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,coolforkit-caps, \
    -Wl$(COMMA)--whole-archive \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib))) \
    -Wl$(COMMA)--no-whole-archive \
    $(UNIX_DLAPI_LIBS) \
    $(if $(filter LINUX,$(OS)),-lrt) \
    $(ONLINE.CAP_LIBS) \
))
$(call gb_Executable_get_target,coolforkit-caps): \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib)))

ifneq ($(OS),MACOSX)
$(eval $(call gb_Executable_add_ldflags,coolforkit-caps,-Wl$(COMMA)-E))
endif

$(eval $(call gb_Executable_add_generated_exception_objects,coolforkit-caps, \
    kit/forkit-variant-caps \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolforkit-caps,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
