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

$(eval $(call gb_Module_add_targets,gbuild, \
    Executable_coconvert \
    Executable_connect \
    Executable_coolconfig \
    Executable_coolconvert \
    Executable_coolmap \
    Executable_coolmount \
    Executable_coolsocketdump \
    Executable_coolstress \
    Executable_lokitclient \
    StaticLibrary_dummytraceevent \
    StaticLibrary_globals \
    StaticLibrary_shared \
))

# vim: set noet sw=4 ts=4:
