# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,liblangtag_data,langtag))

$(eval $(call gb_ExternalPackage_set_outdir,liblangtag_data,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,liblangtag_data,share/liblangtag,\
	data/language-subtag-registry.xml \
	data/common/bcp47/calendar.xml \
	data/common/bcp47/collation.xml \
	data/common/bcp47/currency.xml \
	data/common/bcp47/number.xml \
	data/common/bcp47/timezone.xml \
	data/common/bcp47/transform.xml \
	data/common/bcp47/transform_ime.xml \
	data/common/bcp47/transform_keyboard.xml \
	data/common/bcp47/transform_mt.xml \
	data/common/bcp47/transform_private_use.xml \
	data/common/bcp47/variant.xml \
	data/common/supplemental/likelySubtags.xml \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
