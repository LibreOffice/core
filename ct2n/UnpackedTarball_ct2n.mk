# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,ConvertTextToNumber))

$(eval $(call gb_UnpackedTarball_set_tarball,ConvertTextToNumber,$(CT2N_TARBALL),0))

$(eval $(call gb_UnpackedTarball_add_patches,ConvertTextToNumber,\
	ct2n/ConvertTextToNumber-1.3.2-no-license.patch \
	ct2n/ConvertTextToNumber-1.3.2-no-visible-by-default.patch \
))

$(eval $(call gb_UnpackedTarball_mark_output_file,ConvertTextToNumber,description.xml))

# vim: set noet sw=4 ts=4:
