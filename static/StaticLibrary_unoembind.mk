# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,unoembind))

$(eval $(call gb_StaticLibrary_add_exception_objects,unoembind, \
    static/source/unoembindhelpers/PrimaryBindings \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,unoembind, \
    CustomTarget/static/unoembind/bindings_uno \
))

$(eval $(call gb_StaticLibrary_use_api,unoembind,\
    offapi \
    udkapi \
))

$(call gb_StaticLibrary_get_target,unoembind): $(call gb_CustomTarget_get_target,static/unoembind)

# vim: set noet sw=4 ts=4:
