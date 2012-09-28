# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,wpglib))

$(eval $(call gb_StaticLibrary_use_unpacked,wpglib,wpg))

$(eval $(call gb_StaticLibrary_use_package,wpglib,libwpg_inc))

$(eval $(call gb_StaticLibrary_use_externals,wpglib,\
	wpd \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,wpglib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,wpglib,\
	UnpackedTarball/wpg/src/lib/WPG1Parser \
	UnpackedTarball/wpg/src/lib/WPG2Parser \
	UnpackedTarball/wpg/src/lib/WPGBitmap \
	UnpackedTarball/wpg/src/lib/WPGColor \
	UnpackedTarball/wpg/src/lib/WPGDashArray \
	UnpackedTarball/wpg/src/lib/WPGHeader \
	UnpackedTarball/wpg/src/lib/WPGInternalStream \
	UnpackedTarball/wpg/src/lib/WPGraphics \
	UnpackedTarball/wpg/src/lib/WPGSVGGenerator \
	UnpackedTarball/wpg/src/lib/WPGXParser \
))

# vim: set noet sw=4 ts=4:
