# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriestemplatescommonoffimisc,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/common/offimisc))

$(eval $(call gb_Zip_add_dependencies,accessoriestemplatescommonoffimisc,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriestemplatescommonoffimisc,\
	dummy_common_templates.txt \
))

# vim: set noet sw=4 ts=4:
