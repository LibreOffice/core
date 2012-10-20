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

# Extension class requires description-en-US.txt file
$(eval $(call gb_UnpackedTarball_add_file,ConvertTextToNumber,\
	description-en-US.txt,ct2n/description-en-US.txt))

$(eval $(call gb_UnpackedTarball_add_patches,ConvertTextToNumber,\
	ct2n/ConvertTextToNumber-1.3.2-no-license.patch \
	ct2n/ConvertTextToNumber-1.3.2-no-visible-by-default.patch \
))

# adjustments for using Extension class
# 1. manifest.xml is expected in root directory
# 2. LICENSE file is required, reuse existing COPYING
$(eval $(call gb_UnpackedTarball_set_post_action,ConvertTextToNumber,\
	mv $(call gb_UnpackedTarball_get_dir,ConvertTextToNumber)/META-INF/manifest.xml \
	$(call gb_UnpackedTarball_get_dir,ConvertTextToNumber) && \
	mv $(call gb_UnpackedTarball_get_dir,ConvertTextToNumber)/registration/COPYING \
	$(call gb_UnpackedTarball_get_dir,ConvertTextToNumber)/registration/LICENSE \
))

# vim: set noet sw=4 ts=4:
