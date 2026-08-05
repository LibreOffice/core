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

$(eval $(call gb_Executable_set_generated_cxx_base,coolwsd,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolwsd))

$(eval $(call gb_Executable_set_include,coolwsd, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    -I$(SRCDIR)/../common \
    -I$(SRCDIR)/../net \
    -I$(SRCDIR)/../wsd \
    -I$(SRCDIR)/../kit \
    -I$(SRCDIR)/include \
    $$(INCLUDE) \
    $(online_poco_inc) \
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
    $(call gb_StaticLibrary_get_target,shared) \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib))) \
    -Wl$(COMMA)--no-whole-archive \
    $(UNIX_DLAPI_LIBS) \
    $(if $(filter LINUX,$(OS)),-lrt) \
))
$(call gb_Executable_get_target,coolwsd) : \
    $(call gb_StaticLibrary_get_target,shared) \
    $(foreach lib,$(online_poco_whole_libs),$(call gb_StaticLibrary_get_target,$(lib)))

ifneq ($(OS),MACOSX)
$(eval $(call gb_Executable_add_ldflags,coolwsd,-Wl$(COMMA)-E))
endif

$(eval $(call gb_Executable_add_generated_exception_objects,coolwsd, \
    wsd/AIChatSession \
    wsd/AIUtil \
    wsd/Admin \
    wsd/AdminModel \
    wsd/Auth \
    wsd/COOLWSD \
    wsd/CacheUtil \
    wsd/ClientRequestDispatcher \
    wsd/ClientSession \
    wsd/CollabBroker \
    wsd/CollabFileProxy \
    wsd/CollabSocketHandler \
    wsd/ContentType \
    wsd/DocumentBroker \
    wsd/FileServer \
    wsd/FileServerUtil \
    wsd/HealthCheck \
    wsd/HostUtil \
    wsd/ProofKey \
    wsd/ProxyProtocol \
    wsd/ProxyRequestHandler \
    wsd/QuarantineUtil \
    wsd/RemoteConfig \
    wsd/RequestDetails \
    wsd/RequestVettingStation \
    wsd/ServerAuditUtil \
    wsd/SlideCache \
    wsd/SpecialBrokers \
    wsd/Storage \
    wsd/TileCache \
    wsd/Unzip \
    wsd/coolwsd-fork \
    wsd/dumpWsdState \
    wsd/wopi/CheckFileInfo \
    wsd/wopi/StorageConnectionManager \
    wsd/wopi/WopiProxy \
    wsd/wopi/WopiStorage \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_Executable_add_defs,coolwsd,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
