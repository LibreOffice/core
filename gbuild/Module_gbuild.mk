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

# The online sources.  In the monorepo they sit right above the engine sources,
# but a build from the distribution tarball (which ships no engine) is pointed
# at an engine build tree elsewhere and passes its own source directory in as
# ONLINE.SRCDIR, the same way it passes ONLINE.BUILDDIR.
online_srcdir := $(or $(ONLINE.SRCDIR),$(SRCDIR)/..)

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
    StaticLibrary_wsd \
    StaticLibrary_wsdglobals \
))

# vim: set noet sw=4 ts=4:
