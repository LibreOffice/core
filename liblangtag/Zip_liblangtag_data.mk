# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,liblangtag_data,$(call gb_UnpackedTarball_get_dir,langtag/data)))

$(eval $(call gb_Zip_use_unpacked,liblangtag_data,langtag))

$(eval $(call gb_Zip_add_files,liblangtag_data,\
	language-subtag-registry.xml \
	common/bcp47/calendar.xml \
	common/bcp47/collation.xml \
	common/bcp47/currency.xml \
	common/bcp47/number.xml \
	common/bcp47/timezone.xml \
	common/bcp47/transform.xml \
	common/bcp47/transform_ime.xml \
	common/bcp47/transform_keyboard.xml \
	common/bcp47/transform_mt.xml \
	common/bcp47/transform_private_use.xml \
	common/bcp47/variant.xml \
	common/supplemental/likelySubtags.xml \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
