# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,md4c))

$(eval $(call gb_StaticLibrary_use_unpacked,md4c,md4c))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,md4c))

# $(eval $(call gb_StaticLibrary_add_defs,md4c,-DMD4C_USE_UTF8))

$(eval $(call gb_StaticLibrary_set_include,md4c,\
    -I$(gb_UnpackedTarball_workdir)/md4c/src/ \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,md4c,\
    UnpackedTarball/md4c/src/entity \
    UnpackedTarball/md4c/src/md4c \
    UnpackedTarball/md4c/src/md4c-html \
))

# vim: set noet sw=4 ts=4:
