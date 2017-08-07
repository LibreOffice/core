# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,windows_process))

$(eval $(call gb_StaticLibrary_set_include,windows_process,\
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,windows_process,\
	-DUNICODE \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,windows_process,\
	comphelper/source/windows/windows_process \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
