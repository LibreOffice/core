# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,winextendloaderenv))

$(eval $(call gb_StaticLibrary_use_packages,winextendloaderenv,\
))

$(eval $(call gb_StaticLibrary_add_noexception_objects,winextendloaderenv,\
    desktop/win32/source/extendloaderenvironment \
))

# vim:set noet sw=4 ts=4:
