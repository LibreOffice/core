# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,gbuild))

# Poco static libraries that get linked whole-archive, to keep Poco's static
# initialisers and factory registrations:
online_poco_whole_libs := \
    PocoNetSSL \
    PocoCrypto \
    PocoNet \
    PocoZip \
    PocoUtil \
    PocoJSON \
    PocoXML \
    PocoFoundation

# Consume Poco's headers as system headers, so that gbuild's warning flags do
# not apply to third-party code we do not maintain.  Poco's bundled tsl
# hash-map headers in particular declare locals that shadow a member of their
# private base, which -Wshadow reports afresh for every template instantiation;
# with -I that alone is hundreds of megabytes of output per build.  MSVC proper
# has no -isystem (and gets no -Wshadow either), so keep -I there.
online_poco_inc := \
    $(if $(filter GCC,$(COM))$(COM_IS_CLANG),-isystem,-I) \
    $(gb_UnpackedTarball_workdir)/poco/include

$(eval $(call gb_Module_add_targets,gbuild, \
    Executable_clientnb \
    Executable_coconvert \
    Executable_connect \
    Executable_coolbench \
    Executable_coolconfig \
    Executable_coolconvert \
    Executable_coolforkit-caps \
    Executable_coolforkit-ns \
    Executable_coolmap \
    Executable_coolmount \
    Executable_coolsocketdump \
    Executable_coolstress \
    Executable_coolwsd \
    Executable_lokitclient \
    StaticLibrary_dummytraceevent \
    StaticLibrary_forkit \
    StaticLibrary_globals \
    StaticLibrary_kitglobals \
    StaticLibrary_shared \
    StaticLibrary_simd \
    StaticLibrary_wsdglobals \
))

# vim: set noet sw=4 ts=4:
