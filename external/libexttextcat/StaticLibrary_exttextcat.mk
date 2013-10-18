# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,exttextcat))

$(eval $(call gb_StaticLibrary_use_unpacked,exttextcat,exttextcat))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,exttextcat))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,exttextcat,\
	UnpackedTarball/exttextcat/src/common \
	UnpackedTarball/exttextcat/src/fingerprint \
	UnpackedTarball/exttextcat/src/textcat \
	UnpackedTarball/exttextcat/src/wg_mempool \
	UnpackedTarball/exttextcat/src/utf8misc \
))


# vim: set noet sw=4 ts=4:
