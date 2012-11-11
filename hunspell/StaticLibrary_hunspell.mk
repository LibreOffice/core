# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,hunspell))

$(eval $(call gb_StaticLibrary_use_unpacked,hunspell,hunspell))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,hunspell))

$(eval $(call gb_StaticLibrary_add_defs,hunspell,\
	-DHUNSPELL_STATIC \
	-DOPENOFFICEORG \
))

$(eval $(call gb_StaticLibrary_add_generated_cxxobjects,hunspell,\
	UnpackedTarball/hunspell/src/hunspell/affentry \
	UnpackedTarball/hunspell/src/hunspell/affixmgr \
	UnpackedTarball/hunspell/src/hunspell/dictmgr \
	UnpackedTarball/hunspell/src/hunspell/csutil \
	UnpackedTarball/hunspell/src/hunspell/hashmgr \
	UnpackedTarball/hunspell/src/hunspell/suggestmgr \
	UnpackedTarball/hunspell/src/hunspell/phonet \
	UnpackedTarball/hunspell/src/hunspell/hunzip \
	UnpackedTarball/hunspell/src/hunspell/filemgr \
	UnpackedTarball/hunspell/src/hunspell/replist \
	UnpackedTarball/hunspell/src/hunspell/hunspell \
))

# vim: set noet sw=4 ts=4:
