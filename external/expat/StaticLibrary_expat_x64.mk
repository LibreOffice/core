# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,expat_x64))

$(eval $(call gb_StaticLibrary_set_x64,expat_x64,YES))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,expat_x64))

$(eval $(call gb_StaticLibrary_use_unpacked,expat_x64,expat))

$(eval $(call gb_StaticLibrary_set_include,expat_x64,\
    -I$(call gb_UnpackedTarball_get_dir,expat) \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,expat_x64,\
    -DCOMPILED_FROM_DSP \
))

$(eval $(call gb_StaticLibrary_add_x64_generated_cobjects,expat_x64,\
	UnpackedTarball/expat/lib/xmlparse_x64 \
	UnpackedTarball/expat/lib/xmltok_x64 \
	UnpackedTarball/expat/lib/xmlrole_x64 \
))

# vim: set noet sw=4 ts=4:
