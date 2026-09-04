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

$(eval $(call gb_Executable_Executable,coolwsd))

$(eval $(call gb_Executable_set_visibility_default,coolwsd))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolwsd,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolwsd,$(online_srcdir)))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolwsd))

$(eval $(call gb_Executable_set_include,coolwsd, \
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

$(eval $(call gb_Executable_use_static_libraries,coolwsd, \
    simd \
    wsdglobals \
))

$(eval $(call gb_Executable_use_externals,coolwsd, \
    expat \
    libpng \
    openssl \
    openssl_headers \
    zlib \
    zstd \
))

$(eval $(call gb_Executable_add_libs,coolwsd, \
    -Wl$(COMMA)--whole-archive \
    $(call gb_StaticLibrary_get_target,wsd) \
    $(call gb_StaticLibrary_get_target,shared) \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib))) \
    -Wl$(COMMA)--no-whole-archive \
    $(UNIX_DLAPI_LIBS) \
    $(if $(filter LINUX,$(OS)),-lrt) \
))
$(call gb_Executable_get_target,coolwsd) : \
    $(call gb_StaticLibrary_get_target,wsd) \
    $(call gb_StaticLibrary_get_target,shared) \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib)))

ifneq ($(OS),MACOSX)
$(eval $(call gb_Executable_add_ldflags,coolwsd,-Wl$(COMMA)-E))
endif

$(eval $(call gb_Executable_add_generated_exception_objects,coolwsd, \
    wsd/COOLWSD \
    wsd/coolwsd-fork \
))

# vim: set noet sw=4 ts=4:
