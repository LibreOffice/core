# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libexttextcat))

$(eval $(call gb_StaticLibrary_use_unpacked,libexttextcat,libexttextcat))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,libexttextcat))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libexttextcat,\
	UnpackedTarball/libexttextcat/src/common \
	UnpackedTarball/libexttextcat/src/fingerprint \
	UnpackedTarball/libexttextcat/src/textcat \
	UnpackedTarball/libexttextcat/src/wg_mempool \
	UnpackedTarball/libexttextcat/src/utf8misc \
))


# vim: set noet sw=4 ts=4:
