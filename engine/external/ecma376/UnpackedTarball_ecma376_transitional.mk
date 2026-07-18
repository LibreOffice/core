# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,ecma376_transitional))

$(eval $(call gb_UnpackedTarball_set_tarball,ecma376_transitional,$(ECMA376_TRANSITIONAL_TARBALL),0))

# The archive holds the specification text and two electronic annexes
# side by side; the RELAX NG annex unpacks into the same directory.
$(eval $(call gb_UnpackedTarball_set_pre_action,ecma376_transitional,\
	unzip -qq OfficeOpenXML-RELAXNG-Transitional.zip \
))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,ecma376_transitional,\
	dml-main.rnc \
	sml.rnc \
))

$(eval $(call gb_UnpackedTarball_add_patches,ecma376_transitional,\
	external/ecma376/spreadsheet-text-xml-space.patch \
	external/ecma376/bullet-size-percent-mille.patch \
))

# vim: set noet sw=4 ts=4:
