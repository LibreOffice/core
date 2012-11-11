# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
