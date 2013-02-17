# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,seterror))

$(eval $(call gb_StaticLibrary_add_defs,seterror,\
	-U_DLL \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,seterror,\
	/MT \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,seterror,\
    setup_native/source/win32/customactions/tools/seterror \
))

# vim: set noet sw=4 ts=4:
