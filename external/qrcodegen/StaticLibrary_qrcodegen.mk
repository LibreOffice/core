# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,qrcodegen))

$(eval $(call gb_StaticLibrary_use_unpacked,qrcodegen,qrcodegen))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,qrcodegen,cpp))

$(eval $(call gb_StaticLibrary_use_external,qrcodegen,icu_headers))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,qrcodegen))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,qrcodegen,\
	UnpackedTarball/qrcodegen/cpp/BitBuffer \
	UnpackedTarball/qrcodegen/cpp/QrCode \
	UnpackedTarball/qrcodegen/cpp/QrSegment \
))

# vim: set noet sw=4 ts=4:
