# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,moz_inc,moz_inc))

MOZ_INC = $(subst $(call gb_UnpackedTarball_get_dir,moz_inc),,$(shell find $(call gb_UnpackedTarball_get_dir,moz_inc) -type f))

$(eval $(call gb_ExternalPackage_add_unpacked_files,moz_inc,inc/mozilla,\
	$(MOZ_INC) \
))

# vim: set noet sw=4 ts=4:
