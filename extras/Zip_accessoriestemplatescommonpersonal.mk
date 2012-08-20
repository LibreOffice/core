# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriestemplatescommonpersonal,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/common/personal))

$(eval $(call gb_Zip_add_dependencies,accessoriestemplatescommonpersonal,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriestemplatescommonpersonal,\
	szivesoldal.otg \
))

# vim: set noet sw=4 ts=4:
