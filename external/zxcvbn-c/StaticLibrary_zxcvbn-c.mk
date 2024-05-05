# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zxcvbn-c))

$(eval $(call gb_StaticLibrary_use_unpacked,zxcvbn-c,zxcvbn-c))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,zxcvbn-c))

# zxcvbn-c static library depends on generation of dictionary nodes resulting in dict-src.h
$(eval $(call gb_StaticLibrary_use_external_project,zxcvbn-c,zxcvbn-c,full))

$(eval $(call gb_StaticLibrary_set_include,zxcvbn-c,\
    -I$(gb_UnpackedTarball_workdir)/zxcvbn-c\
    $$(INCLUDE)\
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,zxcvbn-c,\
	UnpackedTarball/zxcvbn-c/zxcvbn \
))

# vim: set noet sw=4 ts=4:
