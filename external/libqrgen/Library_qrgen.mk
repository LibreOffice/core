# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,qrgen))

$(eval $(call gb_Library_use_unpacked,qrgen,libqrgen))

$(eval $(call gb_Library_use_externals,qrgen,\
	revenge \
))

$(eval $(call gb_Library_set_warnings_disabled,qrgen))

$(eval $(call gb_Library_set_include,qrgen,\
    -I$(call gb_UnpackedTarball_get_dir,libqrgen)/cpp/public \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,qrgen,\
	-DDLL_EXPORT \
	-DLIBODFGEN_BUILD \
	-DNDEBUG \
	-DSHAREDPTR_STD \
	-DPACKAGE=\"libqrgen\" \
	-DVERSION=\"0.1.$(QRGEN_VERSION_MICRO)\" \
))

$(eval $(call gb_Library_add_generated_exception_objects,qrgen,\
    UnpackedTarball/libqrgen/cpp/BitBuffer \
	UnpackedTarball/libqrgen/cpp/QrCode \
	UnpackedTarball/libqrgen/cpp/QrSegment \
))

# vim: set noet sw=4 ts=4:
